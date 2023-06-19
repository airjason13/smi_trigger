#include "udp_utils.h"	

int send_udp_packet(char *ip, int port, char *data){
	int udp_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(udp_socket_fd == -1){
		log_error("socket failed!\n");
		return -1;
	}
	// allow multiple sockets to use the same PORT number
    //
    unsigned int yes = 1;
    if (setsockopt(udp_socket_fd, SOL_SOCKET, SO_REUSEADDR, (char*) &yes, sizeof(yes))< 0     ){
       log_error("Reusing ADDR failed");
       return -1;
     }

	struct sockaddr_in dest_addr = {0};
    dest_addr.sin_family = AF_INET;//使用IPv4協議
    dest_addr.sin_port = htons(port);//設置接收方端口號
    dest_addr.sin_addr.s_addr = inet_addr(ip); //設置接收方IP 
	
	int send_len = sendto(udp_socket_fd, data, strlen(data), 0, (struct sockaddr *)&dest_addr,sizeof(dest_addr));
	if (send_len != strlen(data)){
		log_error("send len does not match!\n");
		close(udp_socket_fd);
		return -1;
	}
	close(udp_socket_fd);
	return 0;
}

int send_alive_report(char *ip, int port, char *append_data){
	int ret = -1;
	char send_buf[SEND_DATA_MAX_LEN] = {0};
	if(strlen(append_data) > SEND_DATA_MAX_LEN){
		log_error("data length > SEND_DATA_MAX_LEN");
		return ret;
	}else{
		sprintf(send_buf, "alive;%s", append_data);
		ret = send_udp_packet(ip, port, send_buf);
	}
	return ret;
}

int get_ip_of_interface(char *iface, char *ip)
{
    int fd;
	struct ifreq ifr;
	int iret = -1;
    // replace with your interface name
    // or ask user to input
    
	
	fd = socket(AF_INET, SOCK_DGRAM, 0);

	//Type of address to retrieve - IPv4 IP address
	ifr.ifr_addr.sa_family = AF_INET;

	//Copy the interface name in the ifreq structure
	strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);

	iret = ioctl(fd, SIOCGIFADDR, &ifr);
    if(iret < 0){
        log_error("no ip on %s", iface);
        return iret;
    }
    
	close(fd);
    
    sprintf(ip, "%s", inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr));

    return 0;
}
