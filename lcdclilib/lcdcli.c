#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "lcdcli.h"

#define CLIENT_SOCK_FILE "client.sock"
#define SERVER_SOCK_FILE "/tmp/uds_socket_i2clcd7"

bool b_lcd_active = false;

int set_lcd_active(bool enable)
{
    b_lcd_active = enable;
    return 0;
}

int lcd_send_command(int x, int y, char* cmd)
{
	int fd;
	struct sockaddr_un addr;
	int ret;
	char buff[8192];
	struct sockaddr_un from;
	int ok = 1;
	int len;
    
    if(b_lcd_active == false){
        return -ENODEV;
    }
    
	if ((fd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
		log_error("socket init fail!\n");
		ok = 0;
	}
	//original marked
	/*if( ok ){
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, CLIENT_SOCK_FILE);
		unlink(CLIENT_SOCK_FILE);
		if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
			log_error("socket bind fail!\n");
			ok = 0;
		}
	}*/
	if (ok) {
		memset(&addr, 0, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strcpy(addr.sun_path, SERVER_SOCK_FILE);
		if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
			log_error("connect failed!\n");
			ok = 0;
		}
	}

	if (ok) {
		//strcpy (buff, "iccExchangeAPDU");
		sprintf(buff, "%d:%d:%s", x, y, cmd);
		if (send(fd, buff, strlen(buff)+1, 0) == -1) {
			log_error("send error\n");
			ok = 0;
		}
		//log_debug("sent %s\n", buff);
	}
	if (fd >= 0) {
		close(fd);
	}

	//unlink (CLIENT_SOCK_FILE);
	if ( ok ){
		return 0;
	}else{
		return -1;
	}
}

/*int main(int argc, char* argv[]){
	return send_command(0, 1, "test");
}*/
