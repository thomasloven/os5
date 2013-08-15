#include <serial.h>
#include <arch.h>

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
