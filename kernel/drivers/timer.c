#include <timer.h>
#include <stdint.h>
#include <idt.h>
#include <k_debug.h>

uint32_t ticks;

void timer_init(uint32_t freq)
{
	ticks = 0;

	register_int_handler(IRQ2INT(IRQ_TIMER), timer_tick);

	outb(PIT_CMD_PORT, PIT_USE_CH0 | PIT_ACCESS_LOHI | PIT_MODE_3);
	uint32_t divisor = PIT_DIVISOR_CONST/freq;
	outb(PIT_CH0_DATA_PORT, (divisor & 0xFF));
	outb(PIT_CH0_DATA_PORT, ((divisor >> 8) & 0xFF));
}

registers_t *timer_tick(registers_t *r)
{
	ticks ++;
	if(ticks %10 == 0)
		schedule();
		debug("\n %x", current->tid);
	return r;
}
