void set(int *addr, int value)
{
	addr[0] = value;
}

int main()
{
	int *addr = (int *)0xABCDEF;
	int abc = 123;
	set(addr, abc);
	return 0;
}

void _start()
{
	main();
}
