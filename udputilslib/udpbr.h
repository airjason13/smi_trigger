#ifndef _UDPBR_H_
#define _UDPBR_H_
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
#define CLIENT_ALIVEREPORT_PORT 11333

typedef void (*br_callback_t)(char *from_ip, char *data);


#define UDPBR_CALLBACK_SERVER_ALIVE_REPORT				0
#define UDPBR_CALLBACK_MAX								UDPBR_CALLBACK_SERVER_ALIVE_REPORT 



typedef struct udpbr_params{
	int socketfd;
	int port;
	pthread_t t;
};


int register_udpbr_callback(int func_num, br_callback_t br_callback);
int udpbr_init( int port);

#endif
