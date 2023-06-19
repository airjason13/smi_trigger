#include "udp_cmd.h"	
struct udp_cmd_params ucps;


//static cmd_callback_t cmd_callbacks[CMD_CALLBACK_MAX] = {NULL};

char CMD_NAME_TAG[CMD_CALLBACK_MAX][MAX_CMD_NAME] = {
     CMD_TAG_GET_VERSION,
     CMD_TAG_GET_PICO_NUM,
	 CMD_TAG_GET_CABINET_PARAMS,
     CMD_TAG_GET_FRAME_BRIGHTNESS,
     CMD_TAG_GET_FRAME_BR_DIVISOR,
     CMD_TAG_GET_FRAME_CONTRAST,
     CMD_TAG_GET_FRAME_GAMMA,
     CMD_TAG_GET_PIXEL_INTERVAL,
     CMD_TAG_GET_CLIENT_ID,
	 CMD_TAG_SET_LED_SIZE,
	 CMD_TAG_SET_CABINET_SIZE,
	 CMD_TAG_SET_CABINET_PARAMS,
     CMD_TAG_SET_FRAME_BRIGHTNESS,
     CMD_TAG_SET_FRAME_BR_DIVISOR,
     CMD_TAG_SET_FRAME_CONTRAST,
     CMD_TAG_SET_FRAME_GAMMA,
     CMD_TAG_SET_PIXEL_INTERVAL,
     CMD_TAG_SET_CLIENT_ID,
     CMD_TAG_SET_START_TEST,
	 CMD_TAG_SPEC_TEST,
};



int register_udp_cmd_callback(unsigned int func_num, cmd_callback_t callback){
	log_debug("enter %s\n", __func__);
	if(func_num > CMD_CALLBACK_MAX){
		log_error("callback functions full");
		return -EINVAL;
	}
	if(ucps.udp_cmd_handle[func_num].cmd_callback != NULL){	
		return -EINVAL;			
	}
	ucps.udp_cmd_handle[func_num].cmd_callback = callback;
		
	return 0;
}

void *udp_cmd_thread(void *data){
	struct udp_cmd_params *ucps = (struct udp_cmd_params*)data;
	printf("in %s, ups->port = %d\n", __func__, ucps->port);

	char reply_buf[1024] = {0};
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
       return -1;
    }

	struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // differs from sender
    addr.sin_port = htons(ucps->port);

    // bind to receive address
    //
    if (bind(fd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        perror("bind");
        return;
    }

	while (1) {
        char msgbuf[MSGBUFSIZE];
        int addrlen = sizeof(addr);
		//char reply_buf[1024] = {0};
		int cmd_id = -1;
		int cb_ret = -1;
        int nbytes = recvfrom(fd, msgbuf, MSGBUFSIZE, 0, (struct sockaddr *) &addr, &addrlen);
        if (nbytes < 0) {
            perror("recvfrom");
            return;
        }
        msgbuf[nbytes] = '\0';
        //log_debug("recv %d bytes : %s\n" , nbytes, msgbuf);
		/*Check cmd id*/
		for(int i = CMD_CALLBACK_START; i<CMD_CALLBACK_MAX; i ++){
			if(strstr(msgbuf, ucps->udp_cmd_handle[i].cmd_name)){
				//log_debug("got cmd %s\n", ucps->udp_cmd_handle[i].cmd_name);
				cmd_id = i;
				break;
			}
		}
		if(cmd_id != -1){
			memset(reply_buf, 0, 1024);
			cb_ret = ucps->udp_cmd_handle[cmd_id].cmd_callback(msgbuf, reply_buf);
			//log_debug("cb_ret = %d\n", cb_ret);
		}else{
			continue;
		}
		//log_debug("reply_buf = %s\n", reply_buf);
		//marked for test error handle
		if(sendto(fd, reply_buf, strlen(reply_buf), 0, (struct sockaddr *)     &addr, addrlen) < 0){
			log_error("Could not send datagram!\n");
			continue;
		}
		usleep(3000);
     }
	 close(fd);
	 return;
}


int init_cmd_handle(void){
	for(int i = CMD_CALLBACK_START; i<CMD_CALLBACK_MAX; i ++){
		ucps.udp_cmd_handle[i].cmd_id = i;
		sprintf(ucps.udp_cmd_handle[i].cmd_name, "%s", CMD_NAME_TAG[i]);

	}
}

int udp_cmd_init(int port){
	log_debug("port = %d\n", port);
	
	init_cmd_handle();
	ucps.port = port;
	pthread_create(&ucps.t, NULL, udp_cmd_thread, (void *)&ucps);
	return ucps.t;
}
