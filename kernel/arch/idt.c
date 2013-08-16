#include <idt.h>
#include <stdint.h>
#include <arch.h>
#include <k_debug.h>
#include <thread.h>
#include <string.h>

idt_entry_t idt[NUM_INTERRUPTS];
int_handler_t int_handlers[NUM_INTERRUPTS];
struct idt_pointer idt_p;
extern gdt_entry_t gdt[6];

// ASM isr stubs found in int.S
extern isr_t isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7, isr8, \
         isr9, isr10, isr11, isr12, isr13, isr14, isr15, isr16, isr17, \
         isr18, isr19, isr20, isr21, isr22, isr23, isr24, isr25, isr26, \
         isr27, isr28, isr29, isr30, isr31, isr32, isr33, isr34, isr35, \
         isr36, isr37, isr38, isr39, isr40, isr41, isr42, isr43, isr44, \
         isr45, isr46, isr47, isr128, isr130, isr255;

// List of isrs
isr_t *idt_raw[] = 
{
  &isr0,  &isr1,  &isr2,  &isr3,  &isr4,
  &isr5,  &isr6,  &isr7,  &isr8,  &isr9, //9
  &isr10, &isr11, &isr12, &isr13, &isr14,
  &isr15, &isr16, &isr17, &isr18, &isr19, //19
  &isr20, &isr21, &isr22, &isr23, &isr24,
  &isr25, &isr26, &isr27, &isr28, &isr29, //29
  &isr30, &isr31, &isr32, &isr33, &isr34,
  &isr35, &isr36, &isr37, &isr38, &isr39, //39
  &isr40, &isr41, &isr42, &isr43, &isr44,
  &isr45, &isr46, &isr47, 0,  0, //49
  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0, //59
  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0, //69
  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0, //79
  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0, //89
  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0, //99
  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0, //109
  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0, //119
  0,  0,  0,  0,  0,
  0,  0,  0,  &isr128,  0, //129
  &isr130,  0,  0,  0,  0,
  0,  0,  0,  0,  0, //139
  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0, //149
  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0, //159
  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0, //169
  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0, //179
  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0, //189
  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0, //199
  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0, //209
  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0, //219
  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0, //229
  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0, //239
  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0, //249
  0,  0,  0,  0,  0,
  &isr255
};

void idt_set(uint32_t num, uint32_t base, uint32_t segment, uint8_t flags)
{
  if(!base) return;

  idt_set_base(idt[num], base);
  idt[num].segment = segment;
  idt[num].reserved = 0;
  idt[num].flags = flags;
}

void mask_int(unsigned char int_no)
{
  // Disables interrupt int_no

  uint16_t port;
  uint8_t mask;

  if(int_no < 8)
  {
    port = MPIC_DATA_PORT;
  }
  else
  {
    port = SPIC_DATA_PORT;
    int_no -= 8;
  }

  mask = inb(port) | (1 << int_no);
  outb(port, mask);
}
  
void unmask_int(unsigned char int_no)
{
  // Enable interrupt int_no

  uint16_t port;
  uint8_t mask;

  if(int_no < 8)
  {
    port = MPIC_DATA_PORT;
  }
  else
  {
    port = SPIC_DATA_PORT;
    int_no -= 8;
  }

  mask = inb(port) & ~(1 << int_no);
  outb(port, mask);
}

void idt_init()
{
  // Setup interrupt handling

  // Set up the PIC
  outb(MPIC_CMD_PORT, 0x11);
  outb(SPIC_CMD_PORT, 0x11);
  outb(MPIC_DATA_PORT, 0x20);
  outb(SPIC_DATA_PORT, 0x28);
  outb(MPIC_DATA_PORT, 0x04);
  outb(SPIC_DATA_PORT, 0x02);
  outb(MPIC_DATA_PORT, 0x01);
  outb(SPIC_DATA_PORT, 0x01);

  // Enable all interrupts
  outb(MPIC_DATA_PORT, 0x0);
  outb(SPIC_DATA_PORT, 0x0);

  // Prepare pointer to new IDT
  idt_p.size = (sizeof(idt_entry_t)*NUM_INTERRUPTS) - 1;
  idt_p.offset = (uint32_t)&idt;

  memset(idt,0,sizeof(idt_entry_t)*NUM_INTERRUPTS);
  memset(int_handlers,0,sizeof(int_handler_t)*NUM_INTERRUPTS);

  // Setup IDT with pointers to the ASM stubs
  uint32_t i;
  for(i = 0; i < NUM_INTERRUPTS; i++)
  {
    if(idt_raw[i])
    {
      idt_set(i, (uint32_t)idt_raw[i], SEG_KERNEL_CODE, IDT_PRESENT | IDT_INT_GATE);
    }
  }

  // Syscall interrupts should be callable from userspace
  idt[128].flags |=  IDT_DPL_3;
  idt[255].flags |= IDT_DPL_3;

  // Enable new IDT
  idt_flush((uint32_t)&idt_p);
}

registers_t *idt_handler(registers_t *r)
{
  // Common for all interrupt handlers
  // Should have another name...

  if(ISIRQ(r->int_no))
  {
    // If the interrupt was issued by an IRQ, reset the IRQ handler.
    if(INT2IRQ(r->int_no) > 8)
      outb(SPIC_CMD_PORT, PIC_EOI);
    outb(MPIC_CMD_PORT, PIC_EOI);
  } 

  if(int_handlers[r->int_no])
  {
    // If there is an assigned handler for the interrupt

    // Enable all interrupts but the one we just got
    /* mask_int(INT2IRQ(r->int_no)); */
    /* enable_interrupts(); */

    // Run the assigned interrupts handler
    registers_t *ret = int_handlers[r->int_no](r);

    // If we return to a user mode thread, the TCB needs to be set up
    // appropriately and interrupts should be enabled.
    // Also handle signals here.
    if ((ret->cs & 0x3) == 0x3)
    {
      ret = (registers_t *)handle_signals((thread_t *)ret);
      set_kernel_stack(stack_from_tcb(((thread_t *)ret)));
      ret->eflags |= EFL_INT;
    }

    // Reenable the handled interrupt
    /* unmask_int(INT2IRQ(r->int_no)); */

    return ret;
  } else {
    if(!ISIRQ(r->int_no))
    {
      // If an unhandled interrupt (not IRQ) was issued, halt everything.

      disable_interrupts();
      debug("[error]Unhanded interrupt received, %x\n", r->int_no);
      if(ISIRQ(r->int_no))
        debug(", irq %x", INT2IRQ(r->int_no));
      debug("\n Tid: %x", current->tid);
      print_registers(r);
      for(;;);
    } else {
      return r;
    }
  }

}

int_handler_t register_int_handler(uint32_t num, int_handler_t handler)
{
  // Add interrupt handler to list of interrupt handlers :p

  int_handler_t old = int_handlers[num];
  int_handlers[num] = handler;
  return old;
}

void tss_init()
{
  // Set up the TSS. Very basis setup since the only thing that ever
  // changes is the stack pointer.
  uint32_t base = (uint32_t)&global_tss;
  uint32_t limit = sizeof(tss_t);

  set_gdt_base(gdt[GDT_TSS], base);
  set_gdt_limit(gdt[GDT_TSS], limit);
  gdt[GDT_TSS].flags = 0;
  gdt[GDT_TSS].access = 0x89;

  memset(&global_tss, 0, sizeof(tss_t));
  global_tss.ss0 = SEG_KERNEL_DATA;
  global_tss.iomap = sizeof(tss_t);

  tss_flush(SEG_TSS);
}

