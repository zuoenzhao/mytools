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
	unlink( PATH_WLAN_SCAN_FIFO);
	mkfifo(PATH_WLAN_SCAN_FIFO,0755);
	
	char buf[20] = {0};
	int ret = 0;

	fifo = open(PATH_WLAN_SCAN_FIFO,O_RDONLY|O_NONBLOCK);
	if(fifo < 0)
	{
		perror("open fifo(/var/tmp/test_fifo) failed !\n");
		return -1;
	}
	
	while(1)
	{
		memset(buf, 0, sizeof(buf));
		ret = read(fifo, buf, sizeof(buf));
		if(ret != sizeof(buf))
		{			
			//printf("no date\n");
		}
		else
		{
			printf("have date....[%s]\n", buf);			
		}
		
		sleep(2);
		
	}
	
	
	
	return 0;
}