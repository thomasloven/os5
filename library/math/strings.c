#include <strings.h>
#include <stdint.h>
#include <common.h>

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

int num2str(uint32_t num, uint32_t base, char *buf)
{
	if(num == 0)
	{
		buf[0] = '0';
		buf[1] = '\0';
		return 0;
	}

	uint32_t i=0, j=0;

	while(num > 0)
	{
		if(num%base < 10)
			buf[i++] = (char)((uint32_t)'0' + num%base);
		else
			buf[i++] = (char)((uint32_t)'A' + num%base-10);
		num /= base;
	}

	for(i--, j=0; j<i; j++,i--)
	{
		swap(buf[i],buf[j]);
	}

	buf[i+j+1] = '\0';

	return i+j+1;
}
