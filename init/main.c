#include <syscalls.h>
#include <strings.h>



int main(int argc, char *argv[])
{
	char buffer[255];
	_syscall_printf("Started!");
	num2str(argc, 16, buffer);
	_syscall_printf("\n argc:");
	_syscall_printf(buffer);
	num2str((uintptr_t)argv, 16, buffer);
	_syscall_printf("\n argv:");
	_syscall_printf(buffer);
	_syscall_printf("\n Hello, world!");
	return 0;
}

/*void _start(int a, int b)*/
void _start(int argc, char **argv)
{
	main(argc, argv);
	for(;;);
}
