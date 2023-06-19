#include "udpmr.h"	
struct udpmr_params ups;


static callback_t callbacks[CALLBACK_MAX] = {NULL};

int register_udpmr_callback(int func_num, callback_t callback){
	printf("enter %s\n", __func__);
	if(func_num >= CALLBACK_MAX){
		printf("callback functions full");
		return -1;
	}
	if(callbacks[func_num] != NULL){
		printf("callback function is already assigned!\n");
		return -1;
	}
	callbacks[func_num] = callback;
	return 0;
}

void *udpmr_thread(void *data){
	struct udpmr_params *ups = (struct udpmr_params*)data;
	printf("in %s, ups->port = %d\n", __func__, ups->port);
	printf("in %s, ups->multi_group = %s\n", __func__, ups->multi_group);

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
    addr.sin_port = htons(ups->port);

    // bind to receive address
    //
    if (bind(fd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    // use setsockopt() to request that the kernel join a multicast group
    //
    struct ip_mreq mreq;
	printf("ups->multi_group = %s\n", ups->multi_group);
    mreq.imr_multiaddr.s_addr = inet_addr(ups->multi_group);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &mreq, sizeof(mreq)) < 0){
        perror("setsockopt");
        return 1;
	}
	while (1) {
#if 1
        char msgbuf[MSGBUFSIZE];
        int addrlen = sizeof(addr);
        int nbytes = recvfrom(fd, msgbuf, MSGBUFSIZE, 0, (struct sockaddr *) &addr, &addrlen);
        if (nbytes < 0) {
            perror("recvfrom");
            return 1;
        }
        msgbuf[nbytes] = '\0';
        log_debug("recv %d bytes : %s" , nbytes, msgbuf);
		callbacks[CALLBACK_GET_VERSION](msgbuf);
#else
		log_info("test from udpmr!\n");
#endif
		usleep(30);
     }
	 close(fd);
}


int udpmr_init(char *multi_group, int port){
	printf("multicast_group = %s\n", multi_group);
	printf("port = %d\n", port);
	
	ups.port = port;
	printf("strlen(multi_group) = %d\n", strlen(multi_group));
	memcpy(ups.multi_group, multi_group, strlen(multi_group));
	printf("ups.multi_group = %s\n", ups.multi_group);
	pthread_create(&ups.t, NULL, udpmr_thread, (void *)&ups);

}
