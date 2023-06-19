#ifndef _UDP_CMD_H_
#define _UDP_CMD_H_
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <utildbg.h>
#include <errno.h>
#define MSGBUFSIZE 				1024
#define IP_BUF_SIZE				64

typedef int (*cmd_callback_t)(char *, char *);


//CMD ID
#define CMD_CALLBACK_START				    0
#define CMD_CALLBACK_GET_VERSION 		    0
#define CMD_CALLBACK_GET_PICO_NUM 		    1
#define CMD_CALLBACK_GET_CABINET_PARAMS	    2
#define CMD_CALLBACK_GET_FRAME_BRIGHTNESS   3
#define CMD_CALLBACK_GET_FRAME_BR_DIVISOR   4
#define CMD_CALLBACK_GET_FRAME_CONTRAST     5
#define CMD_CALLBACK_GET_FRAME_GAMMA        6
#define CMD_CALLBACK_GET_PIXEL_INTERVAL     7
#define CMD_CALLBACK_GET_CLIENT_ID          8
#define CMD_CALLBACK_GET_FUNCTION_MAX       8

#define CMD_CALLBACK_SET_LED_SIZE		    CMD_CALLBACK_GET_FUNCTION_MAX + 1 //5
#define CMD_CALLBACK_SET_CABINET_SIZE	    CMD_CALLBACK_GET_FUNCTION_MAX + 2 //6
#define CMD_CALLBACK_SET_CABINET_PARAMS	    CMD_CALLBACK_GET_FUNCTION_MAX + 3 //7
#define CMD_CALLBACK_SET_FRAME_BRIGHTNESS   CMD_CALLBACK_GET_FUNCTION_MAX + 4 //8
#define CMD_CALLBACK_SET_FRAME_BR_DIVISOR   CMD_CALLBACK_GET_FUNCTION_MAX + 5 //9
#define CMD_CALLBACK_SET_FRAME_CONTRAST     CMD_CALLBACK_GET_FUNCTION_MAX + 6 //9
#define CMD_CALLBACK_SET_FRAME_GAMMA        CMD_CALLBACK_GET_FUNCTION_MAX + 7 //10
#define CMD_CALLBACK_SET_PIXEL_INTERVAL     CMD_CALLBACK_GET_FUNCTION_MAX + 8 //11
#define CMD_CALLBACK_SET_CLIENT_ID          CMD_CALLBACK_GET_FUNCTION_MAX + 9 //12
#define CMD_CALLBACK_SET_START_TEST         CMD_CALLBACK_GET_FUNCTION_MAX + 10 //13
#define CMD_CALLBACK_SPEC_TEST			    CMD_CALLBACK_GET_FUNCTION_MAX + 11 //14

#define CMD_CALLBACK_MAX				CMD_CALLBACK_SPEC_TEST
#define CMD_CALLBACK_SIZE				CMD_CALLBACK_MAX + 1

//CMD_NAME_TAG
#define CMD_TAG_GET_VERSION				"get_version"
#define CMD_TAG_GET_PICO_NUM			"get_pico_num"
#define CMD_TAG_GET_CABINET_PARAMS		"get_cabinet_params"
#define CMD_TAG_GET_FRAME_BRIGHTNESS    "get_frame_brightness"
#define CMD_TAG_GET_FRAME_BR_DIVISOR    "get_frame_br_divisor"
#define CMD_TAG_GET_FRAME_CONTRAST      "get_frame_contrast"
#define CMD_TAG_GET_FRAME_GAMMA         "get_frame_gamma"
#define CMD_TAG_GET_PIXEL_INTERVAL      "get_pixel_interval"
#define CMD_TAG_GET_CLIENT_ID           "get_client_id"

#define CMD_TAG_SET_LED_SIZE			"set_led_size"
#define CMD_TAG_SET_CABINET_SIZE		"set_cabinet_size"
#define CMD_TAG_SET_CABINET_PARAMS		"set_cabinet_params"
#define CMD_TAG_SET_FRAME_BRIGHTNESS    "set_frame_brightness"
#define CMD_TAG_SET_FRAME_BR_DIVISOR    "set_frame_br_divisor"
#define CMD_TAG_SET_FRAME_CONTRAST      "set_frame_contrast"
#define CMD_TAG_SET_FRAME_GAMMA         "set_frame_gamma"
#define CMD_TAG_SET_PIXEL_INTERVAL      "set_pixel_interval"
#define CMD_TAG_SET_CLIENT_ID           "set_client_id"
#define CMD_TAG_SET_START_TEST          "set_test_color"
#define CMD_TAG_SPEC_TEST				"spec_test"


#define MAX_CMD_NAME					64

typedef struct udp_cmd_handle{
	int cmd_id;//for callback function id
	char cmd_name[MAX_CMD_NAME];
	int (*cmd_callback)(char *, char *);
	
}udp_cmd_handle_t;


typedef struct udp_cmd_params{
	int socketfd;
	char ip[IP_BUF_SIZE];//ANY
	int port;//recv port
	pthread_t t;//recv cmd thread
	udp_cmd_handle_t udp_cmd_handle[CMD_CALLBACK_SIZE];
};


int register_udp_cmd_callback(unsigned int func_num, cmd_callback_t callback);
int udp_cmd_init( int port);

#endif
