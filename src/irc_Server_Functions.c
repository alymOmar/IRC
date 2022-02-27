#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>

#define MAX_CLIENTS 100
#define BUFFER_SZ 10000

/* Client structure */
typedef struct{
	struct sockaddr_in address;
	int sockfd;
	int c_uid;
	char name[32];
} client_t;

/* Registered nicknames structure */
typedef struct{
	char password[32];
	char email[64];
	char name[32];
} registered_nickname_t;

/* function used in irc_Server_Functions.so that are defined in the main program */
extern void send_message(char *s, int c_uid, int privateUID);
extern int find_client_by_name(char *name);
extern int find_registered_nick(char *name);
extern void command(char *message, client_t *cli, char *buff_out);
extern void queue_remove(int c_uid);

/* trim \n and/or \r */
void str_trim_lf (char* arr, int length){
  int i;
  for (i = 0; i < length; i++) { // trim \n and/or \r
    if (arr[i] == '\n' || arr[i] == '\r') {
      arr[i] = '\0';
      break;
    }
  }
}

/* Add new registered nickname */
void nick_add_func(registered_nickname_t *nick, registered_nickname_t **registered_nickname){
	for(int i=0; i < MAX_CLIENTS; ++i){
		if(!registered_nickname[i]){
			registered_nickname[i] = nick;
			break;
		}
	}
}

/* find registered with nickname */
int find_registered_nick_func(char *name, registered_nickname_t **registered_nickname){
	for(int i = 0; i< MAX_CLIENTS; i++){
		if(registered_nickname[i]){
			if(strcmp(registered_nickname[i]->name, name) == 0){
				return i;
			}
		}
	}
	return -1;
}

/* send the list of online users to the new user */
void send_list(client_t *cli, client_t **clients){
    char buff_out[BUFFER_SZ + 100];
		sprintf(buff_out, "List of users online :\n", cli->name);
		for(int i = 0; i< MAX_CLIENTS; i++){
			if(clients[i]){
				strcat(buff_out, clients[i]->name);
                strcat(buff_out, "\n");
			}
		}
        send_message(buff_out, cli->c_uid, cli->c_uid);
        bzero(buff_out, BUFFER_SZ + 100);
}

/* start the server */
void start_server(int *listenfd, struct sockaddr_in *serv_addr, int *option){

	if(setsockopt(*listenfd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)option,sizeof(*option)) < 0){
		perror("ERROR: setsockopt failed");
        exit(1);
	}

	/* Bind */
    if(bind(*listenfd, (struct sockaddr*)serv_addr, sizeof(*serv_addr)) < 0) {
        perror("ERROR: Socket binding failed");
        exit(1);
    }

    /* Listen */
    if (listen(*listenfd, 10) < 0) {
        perror("ERROR: Socket listening failed");
        exit(1);
    }
}

/* Handle all communication with the client */
void *handle_client(void *arg){
	char message[BUFFER_SZ] = {};
	char buff_out[BUFFER_SZ + 100];
	char name[32];
	int leave_flag = 0;
	time_t s, val = 1;
    struct tm* current_time;

	//cli_count++;
	client_t *cli = (client_t *)arg;

	// ask for name and verify
	sprintf(buff_out, "Please enter your name: ");
	send_message(buff_out, cli->c_uid, cli->c_uid);
	bzero(buff_out, BUFFER_SZ + 100);
	if(recv(cli->sockfd, name, 32, 0) <= 0 || strlen(name) > 31){
		printf("Didn't enter the name.\n");
		leave_flag = 1;
	} else if(strlen(name) <  1){ // give random name
        sprintf(name, "Guest_%d", cli->c_uid);
        strcpy(cli->name, name);
		sprintf(buff_out, "%s has joined\n", cli->name);
		printf("%s", buff_out);
		send_message(buff_out, cli->c_uid, -1);
        sprintf(buff_out, "%s is your name\n", cli->name);
		send_message(buff_out, cli->c_uid, cli->c_uid);
    } else{
		if(find_client_by_name(name) > 0){ // someone with the name is online
			sprintf(buff_out, "%s already is online\n", name);
			send_message(buff_out, cli->c_uid, cli->c_uid);
			leave_flag = 1;
		}else if(find_registered_nick(name) >= 0){ // the name registered
			sprintf(buff_out, "%s is registered! you need to login with password and email\n", name);
			send_message(buff_out, cli->c_uid, cli->c_uid);
			leave_flag = 1;
		} else{ // the name is good
			str_trim_lf(name, strlen(name));
			strcpy(cli->name, name);
			sprintf(buff_out, "%s has joined\n", cli->name);
			printf("%s", buff_out);
			send_message(buff_out, cli->c_uid, -1);
		}
		
	}
	bzero(name, 32);
	bzero(buff_out, BUFFER_SZ + 100);

	while(1){
		if (leave_flag) {
			break;
		}

		int receive = recv(cli->sockfd, message, BUFFER_SZ, 0);
		str_trim_lf(message, strlen(message));
		if (receive > 0){
			if(strlen(message) > 0){
				s = time(NULL);
				current_time = localtime(&s);
                if(strncmp(message, "/", strlen("/")) == 0){ //the messages that start with '/' are commands
					command(message, cli, buff_out);
				} else{ //send message to everyone
					sprintf(buff_out, "%02d:%02d:%02d %s : %s -> EMSE\n", 	current_time->tm_hour,
																			current_time->tm_min,
																			current_time->tm_sec,
																			cli->name,
																			message);
					send_message(buff_out, cli->c_uid, -1);

					str_trim_lf(buff_out, strlen(buff_out));
					printf("%s\n", buff_out);
				}
					
			}
		} else if (receive == 0 || strcmp(message, "exit") == 0){
			sprintf(buff_out, "%s has left\n", cli->name);
			printf("%s", buff_out);
			send_message(buff_out, cli->c_uid, -1);
			leave_flag = 1;
		} else {
			printf("ERROR: -1\n");
			leave_flag = 1;
		}

		bzero(buff_out, BUFFER_SZ + 100);
		bzero(message, BUFFER_SZ);
	}

    /* Delete client from queue and yield thread */
    close(cli->sockfd);
    queue_remove(cli->c_uid);
    free(cli);
    pthread_detach(pthread_self());

    return NULL;
}