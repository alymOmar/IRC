#include "irc_Server_Functions.h" //contain function prototypes of irc_Server_Functions.so

#define MAX_CLIENTS 100
#define BUFFER_SZ 10000
#define IP "10.0.0.30" //ip of server
#define PORT 6665


static _Atomic unsigned int cli_count = 0;
static _Atomic unsigned int nick_count = 0;
static int c_uid = 10;

client_t *clients[MAX_CLIENTS];
registered_nickname_t *registered_nickname[MAX_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Add new connected client */
void queue_add(client_t *cl){
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < MAX_CLIENTS; ++i){
		if(!clients[i]){
			clients[i] = cl;
			break;
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

/* Remove client */
void queue_remove(int c_uid){
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i < MAX_CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->c_uid == c_uid){
				bzero(clients[i]->name, 32);
				clients[i] = NULL;
				break;
			}
		}
	}
	cli_count--;
	pthread_mutex_unlock(&clients_mutex);
}

/* find client with nickname */
int find_client_by_name(char *name){
	for(int i = 0; i< MAX_CLIENTS; i++){
		if(clients[i]){
			if(strcmp(clients[i]->name, name) == 0){
				return clients[i]->c_uid;
			}
		}
	}
	return -1;
}

/* Add registered nickname */
void nick_add(registered_nickname_t *nick){
	nick_add_func(nick, registered_nickname); //function defined in irc_Server_Functions.so
}

/* find registered nickname */
int find_registered_nick(char *name){
	return find_registered_nick_func(name, registered_nickname); //function defined in irc_Server_Functions.so
}


/* Send message to all or a specific clients except sender */
void send_message(char *s, int c_uid, int privateUID){ 
	pthread_mutex_lock(&clients_mutex);

    if(privateUID < 0){ // if privateUID < 0 send to everyone except the sender
        for(int i=0; i<MAX_CLIENTS; ++i){
            if(clients[i]){
                if(clients[i]->c_uid != c_uid){
                    if(write(clients[i]->sockfd, s, strlen(s)) < 0){
                        perror("ERROR: write to descriptor failed");
                        break;
                    }
                }
            }
        }
    }else{ // if privateUID >= 0 send to client with id = privateUID
        for(int i=0; i<MAX_CLIENTS; ++i){
            if(clients[i]){
                if(clients[i]->c_uid == privateUID){
                    if(write(clients[i]->sockfd, s, strlen(s)) < 0){
                        perror("ERROR: write to descriptor failed");
                        break;
                    }
                }
            }
        }
    }

	pthread_mutex_unlock(&clients_mutex);
}

/* verify if the nickname is regitered and if the password an email are correct */
int verify_registered_nick(int n_uid, char* buffPass, char* buffEmail){
	char password[650], email[650];
	strcpy(password, buffPass); //========BUFFER OVERFLOW========//
	strcpy(email, buffEmail); //========BUFFER OVERFLOW========//

	return (n_uid >= 0 && strcmp(registered_nickname[n_uid]->password, password) == 0 && strcmp(registered_nickname[n_uid]->email, email) == 0);
}

/* manage the received commands */
void command(char *message, client_t *cli, char *buff_out){
	char str[BUFFER_SZ] = {};
	strcpy(str, message);	//len(message) == len(str) <= BUFFER_SZ always
	time_t s, val = 1;
    struct tm* current_time;
	s = time(NULL);
	current_time = localtime(&s);
	
	int cmd = 0;
	int nickServ = 0;
	char nickName[BUFFER_SZ] = {}, password[BUFFER_SZ], email[BUFFER_SZ];

	char *ptr = strtok(str, " "); //split the message
	while(ptr != NULL)
	{
		switch (cmd){
			case 0:
				if(strcmp(ptr, "/msg") == 0) cmd = 1;
				else if(strcmp(ptr, "/nick") == 0) cmd = 2;
				else cmd = -1;
				break;
			case 1:
				if(strcmp(ptr, "NickServ") == 0){ //if /msg is followed by NickServ cmd = 3
					cmd = 3;
				}else if(strlen(ptr) < 32){ // else try to send a private message
					int private_c_uid = find_client_by_name(ptr);
					if(private_c_uid > 0){
						strncpy(message, message + strlen("/msg ") + strlen(ptr) + 1, BUFFER_SZ);
						sprintf(buff_out, "%02d:%02d:%02d %s : %s -> %s privatly\n",current_time->tm_hour,
																					current_time->tm_min,
																					current_time->tm_sec,
																					cli->name,
																					message,
																					ptr);
						send_message(buff_out, cli->c_uid, find_client_by_name(ptr));
						str_trim_lf(buff_out, strlen(buff_out));
						printf("%s\n", buff_out);
						return;
					}else{
						sprintf(buff_out, "%s does not exist\n", ptr);
						send_message(buff_out, cli->c_uid, cli->c_uid);
						return;
					}
				}
				break;
			case 2: //change nickname
				if(strlen(ptr) < 32){ //if the nickname is less then 32
					int private_c_uid = find_client_by_name(ptr);
					int private_n_uid = find_registered_nick(ptr);
					if(private_c_uid > 0){
						sprintf(buff_out, "%s is already online! try somthing else\n", ptr);
						send_message(buff_out, cli->c_uid, cli->c_uid);
						return;
					}else if(private_n_uid >= 0){
						sprintf(buff_out, "%s is already registered! try somthing else\n", ptr);
						send_message(buff_out, cli->c_uid, cli->c_uid);
						return;
					} else{
						sprintf(buff_out, "%s changed his nickname to %s\n", cli->name, ptr);
						send_message(buff_out, cli->c_uid, -1);
						printf("%s\n", buff_out);
						bzero(buff_out, BUFFER_SZ + 100);
						sprintf(buff_out, "you have changed your nickname to %s\n", ptr);
						send_message(buff_out, cli->c_uid, cli->c_uid);
						strcpy(cli->name, ptr);
						return;
					}
				}
				break;
			case 3:
				if(nickServ == 0){
					if(strcmp(ptr, "REGISTER") == 0){ //if /msg NickServ is followed by REGISTER
						nickServ = 1;						
					}else if(strcmp(ptr, "IDENTIFY") == 0){ //if /msg NickServ is followed by IDENTIFY
						nickServ = 2;
					}
				}else if(nickServ == 1){ //REGISTER nickname
					if(strlen(password) < 1){
						if(strlen(ptr) < 32){
							strcpy(password, ptr);
						}else{
							sprintf(buff_out, "password too long\n");
							send_message(buff_out, cli->c_uid, cli->c_uid);
							return;
						}
					}else{
						if(strlen(ptr) < 32){
							strcpy(email, ptr);
							registered_nickname_t *nick = (registered_nickname_t *)malloc(sizeof(registered_nickname_t));
							strcpy(nick->name, cli->name);
							strcpy(nick->password, password);
							strcpy(nick->email, email);
							nick_add(nick);
							sprintf(buff_out, "nickname %s is now registered\n", cli->name);
							send_message(buff_out, cli->c_uid, cli->c_uid);
							printf("%s\n", buff_out);
							return;
						}else{
							sprintf(buff_out, "email too long\n");
							send_message(buff_out, cli->c_uid, cli->c_uid);
							return;
						}
						
					}
				}else if(nickServ == 2){ //IDENTIFY nickname
					if(strlen(nickName) < 1){
						if(strlen(ptr) < 32){
							strcpy(nickName, ptr);
						}else{
							sprintf(buff_out, "nickname too long\n");
							send_message(buff_out, cli->c_uid, cli->c_uid);
							return;
						}
					}else if(strlen(password) < 1){
						strcpy(password, ptr);
					}else{
						strcpy(email, ptr);
						int n_uid = find_registered_nick(nickName);

						if(verify_registered_nick(n_uid, password, email)){ // verify the nickname passwprd and email (buffer overflow)
							sprintf(buff_out, "%s changed his nickname to %s\n", cli->name, nickName);
							send_message(buff_out, cli->c_uid, -1);
							printf("%s\n", buff_out);
							bzero(buff_out, BUFFER_SZ + 100);
							sprintf(buff_out, "you have changed your nickname to %s\n", nickName);
							send_message(buff_out, cli->c_uid, cli->c_uid);
							strcpy(cli->name, nickName);
						}else{
							sprintf(buff_out, "nickname, password or email is incorrect\n");
							send_message(buff_out, cli->c_uid, cli->c_uid);
						}

						bzero(password, strlen(password));
						bzero(email, strlen(email));
						bzero(nickName, strlen(nickName));
						return;
					}
				}
				break;
			default:
				sprintf(buff_out, "command not found\n");
				send_message(buff_out, cli->c_uid, cli->c_uid);
				return;
		}
		ptr = strtok(NULL, " ");
		bzero(buff_out, BUFFER_SZ + 100);
	}
	sprintf(buff_out, "command not found\n");
	send_message(buff_out, cli->c_uid, cli->c_uid);	
	
}

int main(int argc, char **argv){
	// start the server
	int option = 1;
	int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    pthread_t tid;

    /* Socket settings */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(IP);
    serv_addr.sin_port = htons(PORT);

	start_server(&listenfd, &serv_addr, &option); //function defined in irc_Server_Functions.so

	printf("=== WELCOME TO THE EMSE CHATROOM ===\n");

	while(1){
		socklen_t clilen = sizeof(cli_addr);
		connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);

		/* Check if max clients is reached */
		if((cli_count + 1) == MAX_CLIENTS){
			printf("Max clients reached. Rejected: ");
			close(connfd);
			continue;
		}

		/* Client settings */
		client_t *cli = (client_t *)malloc(sizeof(client_t));
		cli->address = cli_addr;
		cli->sockfd = connfd;
		cli->c_uid = c_uid++;

		/* Add the new client */
		queue_add(cli);
		cli_count++;
		/* send the list of online users to the new user */
		send_list(cli, clients); //function defined in irc_Server_Functions.so


		pthread_create(&tid, NULL, &handle_client, (void*)cli); //handle_client is defined in irc_Server_Functions.so

		/* Reduce CPU usage */
		sleep(1);
	}

	return EXIT_SUCCESS;
}