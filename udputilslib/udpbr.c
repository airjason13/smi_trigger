#include "udpbr.h"	
struct udpbr_params udpbr_p;


static br_callback_t udpbr_callbacks[UDPBR_CALLBACK_MAX + 1] = {NULL};

int register_udpbr_callback(int func_num, br_callback_t br_callback){
	printf("enter %s\n", __func__);
	if(func_num > UDPBR_CALLBACK_MAX){
		log_error("callback functions full");
		return -1;
	}
	if(udpbr_callbacks[func_num] != NULL){
		log_error("callback function is already assigned!\n");
		return -1;
	}
	udpbr_callbacks[func_num] = br_callback;
	return 0;
}

void *udpbr_thread(void *data){
	struct udpbr_params *udpbr_ps = (struct udpbr_params*)data;
	printf("in %s, ups->port = %d\n", __func__, udpbr_ps->port);
	//printf("in %s, ups->multi_group = %s\n", __func__, udpbr_ps->multi_group);

	int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("error socket:");
        return -1;
    }

    // allow multiple sockets to use the same PORT number
    //
    unsigned int yes = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*) &yes, sizeof(yes))< 0 ){
       perror("Reusing ADDR failed");
       return 1;
    }

	struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // differs from sender
    addr.sin_port = htons(udpbr_ps->port);

    // bind to receive address
    //
    if (bind(fd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

	while (1) {
        char msgbuf[MSGBUFSIZE];
        int addrlen = sizeof(addr);
        int nbytes = recvfrom(fd, msgbuf, MSGBUFSIZE, 0, (struct sockaddr *) &addr, &addrlen);
        if (nbytes < 0) {
            perror("recvfrom");
            return 1;
        }
        msgbuf[nbytes] = '\0';

        //log_debug("recv %d bytes from %s : %s" , nbytes, inet_ntoa(addr.sin_addr) ,msgbuf);
		if(strstr(msgbuf, msgbuf) != NULL){
			//log_debug("run callback server alive report!\n");
			if(udpbr_callbacks[UDPBR_CALLBACK_SERVER_ALIVE_REPORT] != NULL)
				udpbr_callbacks[UDPBR_CALLBACK_SERVER_ALIVE_REPORT](inet_ntoa(addr.sin_addr) , msgbuf);
		}
		usleep(3000);
     }
	 close(fd);
}


int udpbr_init( int port){
	printf("port = %d\n", port);
	
	udpbr_p.port = port;
	pthread_create(&udpbr_p.t, NULL, udpbr_thread, (void *)&udpbr_p);

}
