#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#include <time.h>

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif
#define ifreq_offsetof(x)  offsetof(struct ifreq, x)


int GetEthAttr(const char *inf, int type, char* out)
{
	
	int socketfd;
	struct ifreq struReq;

	memset(&struReq, 0x00, sizeof(struct ifreq));
	strncpy(struReq.ifr_name, inf, sizeof(struReq.ifr_name));
	socketfd = socket(PF_INET, SOCK_STREAM, 0);

	if(type == 1)
	{
		if (-1 == ioctl(socketfd, SIOCGIFADDR, &struReq))
		{
			printf("ioctl ip address error!\n");
			return -1;
		}
		strcpy(out, inet_ntoa(((struct sockaddr_in *)&(struReq.ifr_addr))->sin_addr));
		printf("IpAddress: %s\n", out);
	}

	
	
	else if(type == 2)
	{
		if (-1 == ioctl(socketfd, SIOCGIFHWADDR, &struReq))
		{
			printf("ioctl hwaddr error!\n");
			return -1;
		}
	
		memcpy(out, struReq.ifr_hwaddr.sa_data, 8);
		printf("MacAddress: %02x%02x%02x%02x%02x%02x\n", out[0], out[1], out[2], out[3], out[4], out[5]);
	}
	else if(type == 3)
	{
		if (-1 == ioctl(socketfd, SIOCGIFNETMASK, &struReq))
		{
			printf("ioctl net mask error!\n");
			return -1;
		}
		strcpy((char *)out, inet_ntoa(((struct sockaddr_in *)&(struReq.ifr_netmask))->sin_addr));
		printf("NetMask: %s\n",out);

	}
	else if(type == 4)
	{
		



	}
	else
	{
		printf("please input 1-4.....\n");
	}	

	close(socketfd);

	return 0;

}

int SetEthAttr(const char *ifname, const int type, const char *str)
{
	struct ifreq ifr;
	struct sockaddr_in sai;
	int sockfd;
	int ioctl_cmd = 0;
	char err_str[24] = {0};
	
	
	
	if ((type == 1) || (type == 3))  //ip mask
	{
		if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		{
			printf("Socket Failed\n");
			return -1;
		}
		strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
		
		sai.sin_family = AF_INET;
		sai.sin_port = 0;
		sai.sin_addr.s_addr = inet_addr(str);
		 
		if (sai.sin_addr.s_addr == -1)
		{
			printf("Express Error\n");
			close(sockfd);
			return -1;
		}

		if (type == 1)
		{
			strcpy(err_str, "set ip error\n");
			ioctl_cmd = SIOCSIFADDR;
			memcpy((((char *)(&ifr)) + ifreq_offsetof(ifr_addr)), 
				&sai, sizeof(struct sockaddr));
		}
		else
		{
			strcpy(err_str, "set netmask error\n");
			ioctl_cmd = SIOCSIFNETMASK;
			memcpy((((char *)(&ifr)) + ifreq_offsetof(ifr_netmask)),
				&sai, sizeof(struct sockaddr));
		}


		if (ioctl(sockfd, ioctl_cmd, &ifr) < 0)
		{
			printf("%s\n", err_str);
			close(sockfd);
			return -1;
		}
		
		if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
		{
			printf("SIOCGIFFLAGS Failed\n");
			close(sockfd);
			return -1;
		}

		ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);

		if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0)
		{
			printf("SIOCSIFFLAGS Failed\n");
			close(sockfd);
			return -1;
		}

		if (type == 1)
		{
			printf("Set Ip Successful\n");
		}
		else if(type == 3)
		{
			printf("Set NetMask Successful\n");
		}
		close(sockfd);
		
	}
	else if(type == 4)
	{

		
	}
	else
	{
		printf("Type Error. put 1	3	4\n");
		return -1;
	}

	

	return 0;
}



int main(void)
{
	char ip[32] ={0},mask[32] = {0};
	GetEthAttr("eth2", 1, ip);
	GetEthAttr("eth2", 3, mask);
	memset(ip, 0, 32);
	memset(mask, 0, 32);
	SetEthAttr("eth2", 1, "172.25.123.11");
	GetEthAttr("eth2", 1, ip);
	return 0;
}









