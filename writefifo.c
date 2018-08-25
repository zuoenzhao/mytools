#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>


#define PATH_WLAN_SCAN_FIFO "/var/tmp/wlanscanfifo"

int fifo = -1;

int main(int argc, char *argv[])
{
	char buf[32] = {0};
	int ret = 0;
	
	fifo = open(PATH_WLAN_SCAN_FIFO,O_WRONLY|O_NONBLOCK);
	if(fifo < 0)
	{
		perror("open fifo(/var/tmp/test_fifo) failed !\n");
		return -1;
	}
	
	strcpy(buf, argv[1]);
	
	ret = write(fifo, buf, sizeof(buf));
	if(ret != sizeof(buf))
	{
		printf("write fifo error..\n");
		
	}
	
	else
	{
		printf("write fifo success..\n");		
	}
	
	close(fifo);

	
	
	
	return 0;
}