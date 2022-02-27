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

extern void str_trim_lf (char* arr, int length);
extern void send_list(client_t *cli, client_t **clients);
extern void start_server(int *listenfd, struct sockaddr_in *serv_addr, int *option);
extern void *handle_client(void *arg);
extern int find_registered_nick_func(char *name, registered_nickname_t **registered_nickname);
extern void nick_add_func(registered_nickname_t *nick, registered_nickname_t **registered_nickname);