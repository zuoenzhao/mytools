#include <stdio.h>
 #include <string.h>


int main(void)
{
	char ch[20] = "zhao";
	int a = 0;

	(memcmp(ch, "zhao", 4) == 0)?(a = 5):(a = 1);
	printf("the a = %d.\n", a);


	return 0;
}
