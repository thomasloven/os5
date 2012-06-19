#include <stdint.h>
#include <k_debug.h>

void kinit()
{

	kdbg_init();
	debug("Hello, world!\n%x%d",0x123ABC, 123);


}
