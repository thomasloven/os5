#include <stdint.h>
#include <k_debug.h>
#include <memory.h>
#include <stdarg.h>
#include <common.h>
#include <ctype.h>

uint16_t *vidmem = (uint16_t *)VIDMEM;

uint32_t scrn_x, scrn_y;

uint8_t text_style;

void kdbg_init()
{
	scrn_x = scrn_y = 0;
	text_style = VGA_STYLE(GRAY, BLACK);
	memset((uint8_t *)vidmem, 0x0, SCRN_W*SCRN_H*2);
}

void kdbg_setpos(uint32_t x, uint32_t y)
{
	scrn_x = x;
	scrn_y = y;
}

void kdbg_getpos(uint32_t *x, uint32_t *y)
{
	x[0] = scrn_x;
	y[0] = scrn_y;
}

void kdbg_scroll()
{
	while (scrn_y > SCRN_H-1)
	{
		uint32_t i;
		for(i=0; i < SCRN_W*(SCRN_H-1); i++)
			vidmem[i] = vidmem[i+SCRN_W];
		for(i=SCRN_W*(SCRN_H-1); i<SCRN_W*SCRN_H; i++)
			vidmem[i] = 0x0;
		scrn_y --;
	}
}

void kdbg_putch(uint8_t c, uint8_t style)
{
	vidmem[scrn_y*SCRN_W+scrn_x] = c | (style << 0x8);
}

void kdbg_printf(char *str, ...)
{
	va_list args;
	va_start(args, str);
	char buf[255];
	uint32_t num;
	while(*str)
	{
		if(*str == '\n')
		{
			scrn_x = 0;
			scrn_y++;
		}
		else if(*str == '%')
		{
			str++;
			switch(*str)
			{
				case 'b':
					num = va_arg(args, uint32_t);
					kdbg_num2str(num, 2, buf);
					kdbg_printf(buf);
					kdbg_printf("b");
					break;
				case 'o':
					num = va_arg(args, uint32_t);
					kdbg_num2str(num, 8, buf);
					kdbg_printf("0");
					kdbg_printf(buf);
					break;
				case 'd':
					num = va_arg(args, uint32_t);
					kdbg_num2str(num, 10, buf);
					kdbg_printf(buf);
					break;
				case 'x':
					num = va_arg(args, uint32_t);
					kdbg_num2str(num, 16, buf);
					kdbg_printf("0x");
					kdbg_printf(buf);
					break;
				case 's':
					kdbg_printf(va_arg(args, char *));
					break;
				default:
					str--;
					kdbg_putch(*str, text_style);
					scrn_x++;
			}
		}
		else if(isprint(*str))
		{
			kdbg_putch(*str, text_style);
			scrn_x++;
		}
		else
		{
			;
		}
		if(scrn_x >= SCRN_W)
		{
			scrn_x = 0;
			scrn_y++;
		}
		kdbg_scroll();
		str++;
	}
	va_end(args);

}

int kdbg_num2str(uint32_t num, uint32_t base, char *buf)
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

