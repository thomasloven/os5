#include <syscalls.h>



int main()
{
	_syscall_putch('h');
	_syscall_putch('i');
	return 0;
}

void _start()
{
	main();
}
