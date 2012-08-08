#include <strings.h>
#include <stdint.h>

uint32_t strlen(char *str)
{
	uint32_t i = 0;
	while(str[i++]);
	return i;
}

uint32_t strcmp(char *a, char *b)
{
	uint32_t i = 0;
	while((a[i] == b[i]) && (a[i]!='\0') && (b[i]!='\0'))i++;
	if(a[i] == b[i])
		return 0;
	else
		return i+1;
}
