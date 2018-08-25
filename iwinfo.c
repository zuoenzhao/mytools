#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
//#include <net/if.h>
#include <sys/time.h>
#include <time.h>
#include <sys/select.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <linux/wireless.h>
#include <pthread.h>
#include <sys/mman.h>
#include <fcntl.h>

#define PATH_WLAN_SCAN_FIFO "/var/tmp/wlanscanfifo"	//用于ios配网，先扫描热点，后配网。wlandaemon得知获取热点的状态
#define NETDEV_PROCFILE 	"/proc/net/dev"

typedef struct 
{
	char	SSID[36];
	int nRSSI;              //SEE RSSI_SINGNAL
	int nChannel;
	char  NetType[32];	//Infra, Adhoc
	char  Encode[32];   // NONE, WEP, TKIP, AES
	char  Auth[32];      	// OPEN, SHARED, WEPAUTO, WPAPSK, WPA2PSK, WPANONE, WPA, WPA2
}WLAN_DEVICE;




#if 1
static int GetEthcardState(void)
{
	FILE *fp;
	char temp[256];
		
	if((fp=fopen(NETDEV_PROCFILE,"rb"))==NULL)
	{
        printf("can not open %s.\n", NETDEV_PROCFILE );
        return -1;
	}
	while(fgets(temp,255,fp))
	{  
	
    	temp[strlen(temp) -1] = '\0';
    	if(strstr(temp,"eth2") !=NULL)
  		{
       		printf("net card eth2 has find.\n");

       		fclose(fp);
			return 0;
    	}
	
	} 
	
	printf("net card eth2 has not find.\n");
    fclose(fp);
	
    return -1;
}

WLAN_DEVICE * WLanScanDevices(int *pNum)
{
	static WLAN_DEVICE device[50];//
	static WLAN_DEVICE device_out[50];

	char line[32] = {0};
	char fifobuf[20] = {0};
	char tmp[] = "TSF:"; 		//
	char buf[1024] = {0};		//
	char *p = NULL;
	char *p1 = NULL;
	char *p2 = NULL;
		
	int i = -1;;	
	int ret = -1;
	int devicenum = -1;	//
	int channel = 0;
	int wifiScan_fifo = -1;
		
	if(pNum == NULL)	
	{
		return NULL;
	}

	wifiScan_fifo = open(PATH_WLAN_SCAN_FIFO,O_WRONLY|O_NONBLOCK);
	if(wifiScan_fifo < 0)
	{
		printf("open fifo(/var/tmp/wlanscanfifo) failed !\n");
		return NULL;
	}

	strcpy(fifobuf, "scan_start");

	ret = write(wifiScan_fifo, fifobuf, sizeof(fifobuf));
	if(ret != sizeof(fifobuf))
	{
		printf("write fifo error..\n");
		close(wifiScan_fifo);
		return NULL;
	}	
	
	
	sleep(2);

	//获取无线网卡是处于监听模式还是正常模式
	/*
	while(GetEthcardState() != 0)
	{
		sleep(1);
	}
	*/
	
	
	ret = system("ifconfig eth2 up");//确保搜索有输�?
	if(ret < 0)
	{
		perror("system(\"ifconfig eth2 up\")");
	}
		//通过iwlist eth2 scan 获得输出结果
	
	ret = system("/usr/sbin/iw dev eth2 scan > /tmp/iwlist_tmpinfo");	//覆盖原来的数�?
	if(ret < 0)
	{
		perror("system(\"iw dev eth2 scan > /tmp/iwlist_tmpinfo\")");
	}

	
	FILE *fp=fopen("/tmp/iwlist_tmpinfo","r");
	if(fp==NULL)
	{
		perror("fopen");
		close(wifiScan_fifo);
		return NULL;
	}
	fseek(fp,0,0);
	
	for(i=0;i<3;i++)	//�?次文件，如果3次都为空，出�?
	{ 
		memset(buf,0,sizeof(buf));	
		if(fgets(buf,sizeof(buf),fp)==NULL) 
		{
			sleep(1);	// 1s  中的等待时间	
			fseek(fp,0,0);		
		}
		else
			break;
	}
	
	if(i==3)
	{
		printf("/tmp/iwlist_tmpinfo is empty\n");
		fclose(fp);
		close(wifiScan_fifo);
		return NULL;
	}
	

	memset(device,0,sizeof(device));
	devicenum = -1;	
	
	fseek(fp,0,0);

	memset(device, 0, sizeof(device));
	memset(device_out, 0, sizeof(device));


	
	while(1)
	{
		memset(buf,0,sizeof(buf));
		if(fgets(buf,sizeof(buf),fp)!=NULL)//读取一行数�?
		{
			
		
			if(NULL!=(p=strstr(buf,tmp)))
			{	
				devicenum++;
				if(devicenum > 49)
				{
					break;
				}
				strcpy(device[devicenum].Encode,"NONE");
				strcpy(device[devicenum].Auth,"OPEN");
				
			}

			else if(NULL!=(p=strstr(buf,"freq:")))
			{
				channel = atoi(p+5);
				//printf("\nthe chanel is %d\n", channel);
				if(channel ==2484)
				{
					device[devicenum].nChannel = 14;
				}
				else if(channel == 2504)
				{
					device[devicenum].nChannel = 15;
				}
				else if((channel > 2407) && (channel < 2484))
				{
					device[devicenum].nChannel = (channel-2407)/5;
				}
				else
				{
					device[devicenum].nChannel = 0;
				}

			}
			else if(NULL!=(p=strstr(buf,"capability:")))
			{
				p=strstr(buf,"IBSS");
				if(NULL != p)
				{
					strcpy(device[devicenum].NetType,"Adhoc");
				}
				else
				{
					strcpy(device[devicenum].NetType,"Infra");
				}				

			}
			else if(NULL!=(p=strstr(buf,"signal:")))
			{
				p=strstr(buf,"-");
				//device[devicenum].nRSSI = 100 - atoi(p+1);//实质上值越小信号越好�?
				device[devicenum].nRSSI = 0-atoi(p+1);//实质上值越小信号越好�?
				//printf("device[%d].nRSSI = %d\n",devicenum,device[devicenum].nRSSI);
			}
			
			else if(NULL!=(p=strstr(buf,"SSID:")))
			{
				p += 5;
				int len = strlen(p);
				//printf("len = %d\n",len);
				memset(device[devicenum].SSID, 0, sizeof(device[devicenum].SSID));
				if(len > 32)
				{
					strncpy(device[devicenum].SSID,"Null",4);

				}
				else
					strncpy(device[devicenum].SSID,p,len-1);
				//device[devicenum].SSID[len -1] = 0;
				//printf("device[%d].SSID = %s[%d]\n",devicenum,device[devicenum].SSID,len);
			}		
			else if(NULL!=(p=strstr(buf,"Pairwise")))
			{
				
				p1=strstr(buf,"CCMP");
				p2 = strstr(buf, "WEP");
				p = strstr(buf,"TKIP");

				if((p1 != NULL) && (p != NULL) && (p2 == NULL))
				{
					strcpy(device[devicenum].Encode,"AES+TKIP");
					strcpy(device[devicenum].Auth,"WPA2PSK");
				}
				else if((p1 != NULL) && (p == NULL) && (p2 == NULL))
				{
					strcpy(device[devicenum].Encode,"AES");
					strcpy(device[devicenum].Auth,"WPA2PSK");
				}
				else if((p1 == NULL) && (p != NULL) && (p2 == NULL))
				{
					strcpy(device[devicenum].Encode,"TKIP");
					strcpy(device[devicenum].Auth,"WPAPSK");
				}
				else if((p1 == NULL) && (p == NULL) && (p2 != NULL))
				{
					strcpy(device[devicenum].Encode,"WEP");
					strcpy(device[devicenum].Auth,"SHARED");
				}			

				
			}		

			
			
			
		
		}
		else
		{			
			break;
		}
	}

	p = NULL;	
	p1 = NULL;
	p2 = NULL;
	

	//获取列表成功，告诉wlandaemon获取成功，可以正常进入配网模式
	memset(fifobuf, 0, sizeof(fifobuf));
	strcpy(fifobuf, "scan_end");

	

		
	fclose(fp);
	

	printf("mike devicenum is %d \n",devicenum +1);
	
	
	
		//devicenum = 3;
	*pNum = devicenum +1;


	int ii;
	int null = 0;	//有识别到的ssid大小大于33时，iw出来的是乱码的，去掉这个ssid
	//memset(device_out, 0, sizeof(device));
	for(ii=0;ii<(devicenum+1) ;ii++)
	{
		#if 0
		printf("num:%-10d	", ii);
		printf("SSID:%-32s",device[ii].SSID);
		printf("nChannel=%-10d", device[ii].nChannel);
		printf("nRSSI = %-10d",device[ii].nRSSI);
		printf("Encode:%-15s",device[ii].Encode);
		printf("Auth:%-15s",device[ii].Auth);
		printf("NetType:%-20s\n", device[ii].NetType);
		#endif
	

		if(memcmp(device[ii].SSID, "Null", 4) == 0)
		{
			null++;
			continue;			
		}

		memcpy(device_out[ii - null].SSID, device[ii].SSID, strlen(device[ii].SSID));
		memcpy(device_out[ii - null].Encode, device[ii].Encode, strlen(device[ii].Encode));	
		memcpy(device_out[ii - null].Auth, device[ii].Auth, strlen(device[ii].Auth));
		memcpy(device_out[ii - null].NetType, device[ii].NetType, strlen(device[ii].NetType));
		device_out[ii - null].nRSSI = device[ii].nRSSI;
		device_out[ii - null].nChannel = device[ii].nChannel;
		
	}

	#if 0
	printf("\n\n\n");
	for(ii=0;ii<(devicenum +1 -null);ii++)
	{
		
		printf("num:%-10d	", ii);
		printf("SSID:%-32s",device_out[ii].SSID);
		printf("nChannel=%-10d", device_out[ii].nChannel);
		printf("nRSSI = %-10d",device_out[ii].nRSSI);
		printf("Encode:%-15s",device_out[ii].Encode);
		printf("Auth:%-15s",device_out[ii].Auth);
		printf("NetType:%-20s\n", device_out[ii].NetType);

	}
	#endif


	printf("mike out devicenum is %d \n",devicenum +1-null);
	*pNum = devicenum +1 -null;	
	
	ret = write(wifiScan_fifo, fifobuf, sizeof(fifobuf));
	if(ret != sizeof(fifobuf))
	{
		printf("write fifo error..\n");
		close(wifiScan_fifo);
		return NULL;
	}
	close(wifiScan_fifo);
	
		
	return (device_out);	

}
#endif



int main(int argc, char *argv[])
{
	/*
	Gpio_State_s state;

	//xm510寄存器操作函�?
	memset(&state, 0, sizeof(state));
	state.port = atoi(argv[1]);
	state.bit = atoi(argv[2]);
	state.mulVal = atoi(argv[3]);
	state.dirVal = atoi(argv[4]);
	state.type = atoi(argv[5]);
	if(state.type == 0)
	{
		xmSysRegHandle(state, 0);
	}		
	else if(state.type == 1)
	{
		xmSysRegHandle(state, atoi(argv[6]));
	}
	*/
	int num = 0;
	WLAN_DEVICE device[32];
	memset(device, 0, sizeof(device));
	
	WLanScanDevices(&num);

	printf("the num = %d\n", num);

	
	

	return 0;
}
