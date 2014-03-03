#pragma once

#define SCANCODE_CTRL 0x1D
#define SCANCODE_LSHIFT 0x2A
#define SCANCODE_RSHIFT 0x36
#define SCANCODE_ALT 0x38
#define SCANCODE_CAPSLOCK 0x3A

int kbd_shift;
int kbd_ctrl;
int kbd_alt;

void keyboard_init();
unsigned char keyboard_decode(unsigned char scancode);
