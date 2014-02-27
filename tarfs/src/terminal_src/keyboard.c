#include "keyboard.h"


int kbd_map[128] = 
{
  -1, '\033', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
  0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
  0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
  '*',
  '0',
  ' ',
  0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //F-keys
  0,
  0,
  '7', '8', '9',
  '-',
  '4', '5', '6',
  '+',
  '1', '2', '3',
  '0',
  '.',
  -1, -1, -1,
  0, //F11
  0, //F12
  -1
};

int kbd_mapS[128] = 
{
  -1, '\033', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
  '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
  0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
  0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
  '*',
  '0',
  ' ',
  0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0,
  0,
  '7', '8', '9',
  '-',
  '4', '5', '6',
  '+',
  '1', '2', '3',
  '0',
  '.',
  -1, -1, -1,
  0,
  0,
  -1
};

int kbd_multi;

void keyboard_init()
{
  kbd_shift = 0;
  kbd_ctrl = 0;
  kbd_alt = 0;
  kbd_multi = 0;
}

unsigned char keyboard_decode(unsigned char scancode)
{
  unsigned char retval = 0;

  if(scancode & 0x80)
  {

    // Key up
    switch(scancode & 0x7F)
    {
      case 0x60: // 0xE0 & 0x7F
        break;
      case SCANCODE_CAPSLOCK:
      case SCANCODE_CTRL:
        kbd_ctrl = 0;
        break;

      case SCANCODE_ALT:
        kbd_alt = 0;
        break;

      case SCANCODE_LSHIFT:
      case SCANCODE_RSHIFT:
        kbd_shift = 0;
        break;

      default:
        break;
    }

  } else {

    // Key down
    switch(scancode)
    {
      case SCANCODE_CAPSLOCK:
      case SCANCODE_CTRL:
        kbd_ctrl = 1;
        break;

      case SCANCODE_ALT:
        kbd_alt = 1;
        break;

      case SCANCODE_LSHIFT:
      case SCANCODE_RSHIFT:
        kbd_shift = 1;
        break;

      default:
        if (kbd_map[scancode] == -1)
          break;
        if(kbd_shift)
          retval = (unsigned char)kbd_mapS[scancode];
        else
          retval = (unsigned char) kbd_map[scancode];
        break;
    }

  }

  if(scancode == 0xE0)
    kbd_multi = 1;
  else
    kbd_multi = 0;
  return retval;
}
