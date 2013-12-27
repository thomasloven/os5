#pragma once

#define SERIAL_COM1 0x3f8
#define SERIAL_COM2 0x2f8
#define SERIAL_COM3 0x3e8
#define SERIAL_COM4 0x2e8

void init_serial(short port);
void serial_send(short port, char c);
void serial_debug(char *str, ...);
