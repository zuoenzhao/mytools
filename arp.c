#include <stdio.h>
#include <string.h>
#include <sys/select.h>     
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>
#include <netpacket/packet.h>
#include <linux/if_ether.h>
#include <net/if_arp.h>




#define u_char unsigned char
#define __be16 unsigned short


struct    ether_header {
    unsigned char    ether_dhost[6];
    unsigned char    ether_shost[6];
    unsigned short    ether_type;
};

unsigned char SetIpAddress(char *pInterface,unsigned int IpAddress)
{

    struct ifreq ifr;
    struct sockaddr_in *sinaddr;
    int sockfd;

    
    if((pInterface == NULL)||(pInterface[0]!='e')||(pInterface[1]!='t')||(pInterface[2]!='h'))
    {
        printf("input inteface error!\n");
        return -1;
    }
    if(IpAddress == 0)
    {
        printf("input ip address error\n");
        return -1;
    }
 
    sockfd= socket(AF_INET,SOCK_DGRAM,0);
	if (sockfd<0)
	{
		printf("Can't creat socket  \r\n");
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, pInterface, sizeof(ifr.ifr_name)-1);
	sinaddr = (struct sockaddr_in *)&ifr.ifr_addr;
	sinaddr->sin_family = AF_INET;
	sinaddr->sin_addr.s_addr = IpAddress;
	if (0 > ioctl(sockfd, SIOCSIFADDR, &ifr)) 
	{
		printf("Can't set ip address\r\n");
	}

    close(sockfd);

    return 1;
}

struct ether_arp{
    struct arphdr ea_hdr; //ARPfixed-size header(ARP固定大小的报头)-8字节
    u_char arp_sha[6]; //sender hardware address(发送端硬件地址)-6字节
    u_char arp_spa[4]; //sender protocol address(发送端协议地址)-4字节
    u_char arp_tha[6]; // target hardware address(接收端硬件地址)-6字节
    u_char arp_tpa[4]; //target protocol address(接收端协议地址)-4字节
};

typedef struct _tagARP_PACKET{    
struct ether_header  eh;    
struct ether_arp arp;    
}ARP_PACKET_OBJ, *ARP_PACKET_HANDLE;   



int GetHwAttr(const char *ifname, int type, char* out)
{
	int socketfd;
	struct ifreq struReq;
	

	memset(&struReq, 0x00, sizeof(struct ifreq));
	strncpy(struReq.ifr_name, ifname, sizeof(struReq.ifr_name));
	
	socketfd = socket(PF_INET, SOCK_STREAM, 0);
	if (-1 == ioctl(socketfd, SIOCGIFHWADDR, &struReq))
	{
		printf("ioctl hwaddr error!\n");
		return -1;
	}

	memcpy(out, struReq.ifr_hwaddr.sa_data, 8);
	

	return 0;

}

static int recv_arp(int sockfd, struct sockaddr_ll *peer_addr,unsigned char* src_ip)  
{  
	int i = 0, ret = 0;
	int rtval;  
	ARP_PACKET_OBJ frame;  
	fd_set fdRead;
	struct timeval tv;

	FD_ZERO(&fdRead);
	FD_SET(sockfd,&fdRead);
	tv.tv_sec = 2;
   	tv.tv_usec = 0;
	ret = select(sockfd+1, &fdRead, NULL, NULL, &tv);
	if(ret<0)
	{
		printf("select error...\n");
		
		
	}
	else if (ret == 0 )
	{
		printf("select timeout\n");
		
	}

	printf("the FD_ISSET is %d.\n", FD_ISSET(sockfd, &fdRead));
	
	if(FD_ISSET(sockfd, &fdRead))
	{
		memset(&frame, 0, sizeof(ARP_PACKET_OBJ));  
		rtval = recvfrom(sockfd, &frame, sizeof(frame), 0,NULL, NULL);  
		if (htons(2) == frame.arp.ea_hdr.ar_op ) 
		{
			if(rtval > 0)
			{  				
				if (memcmp(frame.arp.arp_spa, src_ip, 4) == 0)  
				{  
					printf( "IP address is common~\n");  
					for(i=0;i<sizeof(frame.eh.ether_shost);i++)
						printf("%x ",frame.eh.ether_shost[i]);
					printf("\n");
					return 0;  
				}  
			}  
		}
		printf("not recv any info.\n");

	}

	printf("not recv any info.\n");	
		
	
	return -1;  
}


static int send_arp(int sockfd, struct sockaddr_ll *peer_addr,const unsigned char* dst_ip)  
{
	int rtval;  
	ARP_PACKET_OBJ frame;  
	char dst_mac_hdr[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
	char dst_mac[6] = {0x0,0x0,0x0,0x0,0x0,0x0};
	char src_mac[12] = {0};
	unsigned char src_ip[4] = {0};
	memcpy(src_ip, dst_ip, 4);
	if (GetHwAttr("eth2", 1, src_mac) != 0)
	{
		printf("Get HwAddr failed\n");
		return 0;
	}
	memset(&frame, 0x00, sizeof(ARP_PACKET_OBJ));  

	memcpy(frame.eh.ether_dhost, dst_mac_hdr, 6);    
	memcpy(frame.eh.ether_shost, src_mac, 6);    
	frame.eh.ether_type = htons(ETH_P_ARP);      

	frame.arp.ea_hdr.ar_hrd = htons(ARPHRD_ETHER);  
		
	frame.arp.ea_hdr.ar_pro = htons(ETH_P_IP); 
	frame.arp.ea_hdr.ar_hln = 6;                
	frame.arp.ea_hdr.ar_pln = 4;                
	frame.arp.ea_hdr.ar_op = htons(ARPOP_REQUEST);  
	memcpy(frame.arp.arp_sha, src_mac, 6);
	memcpy(frame.arp.arp_spa, src_ip, 4);
//	memset(frame.arp.arp_spa,0,4);
	memcpy(frame.arp.arp_tha, dst_mac_hdr, 6);
	memcpy(frame.arp.arp_tpa, dst_ip,4);
	
	printf("the src mac is %x %x %x %x %x %x\n", frame.arp.arp_sha[0], frame.arp.arp_sha[1], frame.arp.arp_sha[2],
											frame.arp.arp_sha[3],frame.arp.arp_sha[4],frame.arp.arp_sha[5]);
	printf("the src ip is %hhu %hhu %hhu %hhu\n", frame.arp.arp_spa[0], frame.arp.arp_spa[1], frame.arp.arp_spa[2], frame.arp.arp_spa[3]);
	printf("the dst mac is %x %x %x %x %x %x\n", frame.arp.arp_tha[0], frame.arp.arp_tha[1], frame.arp.arp_tha[2],
											frame.arp.arp_tha[3],frame.arp.arp_tha[4],frame.arp.arp_tha[5]);
	printf("the dst ip is %hhu %hhu %hhu %hhu\n", frame.arp.arp_tpa[0], frame.arp.arp_tpa[1], frame.arp.arp_tpa[2], frame.arp.arp_tpa[3]);

	rtval = sendto(sockfd, &frame, sizeof(ARP_PACKET_OBJ), 0,(struct sockaddr*)peer_addr, sizeof(struct sockaddr_ll));    
	if (rtval < 0) {  
		printf("send arp error");
		return -1;  
	} 
	return 0;  
}


static int IsIpConflict(char* device , char* ip)  
{ 
	int sockfd;  
	int rtval = -1;  
	int check_cnt = 3;
	struct sockaddr_ll peer_addr;  
	unsigned char src_ip[4]={0};
	unsigned char dst_ip[4]={0};
	if(ip == NULL){
		printf("ip can't be null");
		return -1;
	}
	sscanf(ip,"%hhu.%hhu.%hhu.%hhu",&src_ip[0],&src_ip[1],&src_ip[2],&src_ip[3]);
	memcpy(dst_ip,src_ip,4);
	printf("%hhu %hhu %hhu %hhu\n",src_ip[0],src_ip[1],src_ip[2],src_ip[3]);
//	printf("%hhd %hhd %hhd %hhd\n",dst_ip[0],dst_ip[1],dst_ip[2],dst_ip[3]);

	sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));  
	if (sockfd < 0) {  
		printf("sock error");
	} 

	memset(&peer_addr, 0, sizeof(peer_addr));    
	peer_addr.sll_family = AF_PACKET;    
	struct ifreq req;  
	bzero(&req, sizeof(struct ifreq));  
	strcpy(req.ifr_name, device);    
	if(ioctl(sockfd, SIOCGIFINDEX, &req) != 0){
		perror("ioctl()");    
	}
	peer_addr.sll_ifindex = req.ifr_ifindex;    
	peer_addr.sll_protocol = htons(ETH_P_ARP);  
	bind(sockfd, (struct sockaddr *) &peer_addr, sizeof(peer_addr));

	while(check_cnt--) 
	{ 
		rtval = send_arp(sockfd, &peer_addr,dst_ip);  
		if ( rtval < 0) {  
			printf("send arp error");
		}  
		rtval = recv_arp(sockfd, &peer_addr,src_ip);  
		if (rtval == 0) {  
			printf ("Get packet from peer and IP conflicts!\n");  
			return 1;
		} else if (rtval < 0) {  
			fprintf(stderr, "Recv arp IP not conflicts: %s\n", strerror(errno));  
		} else {  
			printf("recv arp error");
		}  
	}  
	close(sockfd);
	return 0;  
}


int main(int argc, char *argv[])
{
	
	SetIpAddress("eth2", inet_addr(argv[1]));
	IsIpConflict("eth2", argv[1]);
	
	
	
	
	return 0;
	
}