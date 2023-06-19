#ifndef _UDPMR_H_
#define _UDPMR_H_
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <utildbg.h>

#define MSGBUFSIZE 				256
#define MULTICAST_GROUP_STRLEN 	64

typedef void (*callback_t)(char *);

#define CALLBACK_GET_VERSION 		0
#define CALLBACK_GET_TEST			1

#define CALLBACK_MAX				CALLBACK_GET_TEST



typedef struct udpmr_params{
	int socketfd;
	char multi_group[MULTICAST_GROUP_STRLEN];
	int port;
	pthread_t t;
};


int register_udpmr_callback(int func_num, callback_t callback);
int udpmr_init(char *multi_group, int port);

#endif
