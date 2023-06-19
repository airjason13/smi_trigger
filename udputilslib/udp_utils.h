#ifndef _UDP_UTILS_H_
#define _UDP_UTILS_H_
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <utildbg.h>



#define SEND_DATA_MAX_LEN		1024

int send_alive_report(char *ip, int port, char *append_data);
int get_ip_of_interface(char *iface, char *ip);
#endif
