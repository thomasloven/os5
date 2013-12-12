#include <keyboard.h>
#include <idt.h>
#include <arch.h>
#include <vfs.h>
#include <k_debug.h>

#include <stdio.h>
#include <fcntl.h>

unsigned char kbd_map[128] = 
{
  0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
  0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
  0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
  '*',
  '0',
  ' ',
  0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0,
  0,
  0,
  0,
  0,
  '-',
  0,
  0,
  0,
  '+',
  0,
  0,
  0,
  0,
  0,
  0, 0, 0,
  0,
  0,
  0
};

unsigned char kbd_mapS[128] = 
{
  0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
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
  0,
  0,
  0,
  '-',
  0,
  0,
  0,
  '+',
  0,
  0,
  0,
  0,
  0,
  0, 0, 0,
  0,
  0,
  0
};

INODE keyboard_pipe;
INODE keyboard_raw;
char kbd_state = 0;

unsigned char keyboard_decode(unsigned char scancode)
{
  if(scancode & 0x80)
  {
    // Release key
    scancode = scancode & 0x7F;
    if(scancode == 0x2A || scancode == 0x36)
    {
      // Release shift
      kbd_state = 0;
    }
    return 0;
  } else {
    // Press key
    if(scancode == 0x2A || scancode == 0x36)
    {
      // Shift key
      kbd_state = 1;
      return 0;
    }
    
    if(kbd_state == 1)
      return kbd_mapS[scancode];
    else
      return kbd_map[scancode];
  }
}

registers_t *keyboard_handler(registers_t *r)
{
  unsigned char code[2] = { 0, '\0'};
  unsigned char scancode;
  while(inb(KBD_STATUS_PORT) & 0x2);

  scancode = inb(KBD_DATA_PORT);
  
  // Write ascii to /dev/kbd and the raw scancode
  // to /dev/kbdraw
  code[0] = keyboard_decode(scancode);
  if(code[0])
  {
    vfs_write(keyboard_pipe, (char *)code, 1, 0);
    // Echo key to stdout
    fputc((int)code[0], stdout);
    fflush(stdout);
  }
  code[0] = scancode;
  vfs_write(keyboard_raw, (char *)code, 1, 0);

  return r;
}

void keyboard_init()
{
  keyboard_pipe = new_pipe(1024);
  /* vfs_mount("/dev/kbd", keyboard_pipe); */
  /* vfs_open(keyboard_pipe, O_WRONLY); */

  keyboard_raw = new_pipe(1024);
  /* vfs_mount("/dev/kbdraw", keyboard_raw); */
  /* vfs_open(keyboard_raw, O_WRONLY); */

  register_int_handler(IRQ2INT(IRQ_KBD), keyboard_handler);

}

