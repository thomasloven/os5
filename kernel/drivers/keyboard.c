#include <keyboard.h>
#include <idt.h>
#include <arch.h>
#include <vfs.h>
#include <k_debug.h>

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

fs_node_t *keyboard_pipe;

registers_t *keyboard_handler(registers_t *r)
{

  while(inb(KBD_STATUS_PORT) & 0x2);
  unsigned char scancode = inb(KBD_DATA_PORT);
  if(scancode & 0x80)
  {
  } else {
    unsigned char code[2];
    code[0] = kbd_map[scancode];
    code[1] = '\0';
    debug(code);
    vfs_write(keyboard_pipe, 0, 1, (char *)code);
  }

  return r;
}

void keyboard_init()
{

  keyboard_pipe = new_pipe(1024);
  vfs_mount("/dev/kbd", keyboard_pipe);

  register_int_handler(IRQ2INT(IRQ_KBD), keyboard_handler);

}

