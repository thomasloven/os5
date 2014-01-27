#include <serial.h>
#include <arch.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void init_serial(short port)
{
  outb((short)(port + 1), 0x00);
  outb((short)(port + 3), 0x80);
  outb((short)(port + 0), 0x03);
  outb((short)(port + 1), 0x00);
  outb((short)(port + 3), 0x03);
  outb((short)(port + 2), 0xC7);
  outb((short)(port + 4), 0x0B);
}

void serial_send(short port, char c) {
  while (!(inb((short)(SERIAL_COM1 + 5))&0x20));

  outb(port, c);
}

extern int debug_enabled;
void serial_debug(char *str, ...)
{
  if(!debug_enabled)
    return;
  va_list args;
  va_start(args, str);

  char *buf;

  vasprintf(&buf, str, args);
  unsigned int i;
  for(i = 0; i < strlen(buf); i++)
    serial_send(SERIAL_COM1, buf[i]);

  free(buf);
  va_end(args);
}


