#include <stdio.h>
#inlcude <string.h>


typedef struct 
{
	char	SSID[36];
	int nRSSI;              //SEE RSSI_SINGNAL
	int nChannel;
	char  NetType[32];	//Infra, Adhoc
	char  Encode[32];   // NONE, WEP, TKIP, AES
	char  Auth[32];      	// OPEN, SHARED, WEPAUTO, WPAPSK, WPA2PSK, WPANONE, WPA, WPA2
}WLAN_DEVICE;

extern WLAN_DEVICE * WLanScanDevices(int *pNum);

int mian(void)
{
	WLAN_DEVICE device[50];
	int num = 0;
	
	device = WLanScanDevices(&num);
	
	printf("the num is %d\n", num);
	
	return 0;
	
}