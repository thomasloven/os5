#include <k_syscalls.h>
#include <syscalls.h>
#include <thread.h>
#include <process.h>
#include <k_debug.h>
#include <elf.h>
#include <memory.h>
#include <heap.h>
#include <strings.h>
#include <idt.h>
#include <vmm.h>

KDEF_SYSCALL(fork, r)
{
	fork_process(current->proc, r);
	return r;
}

KDEF_SYSCALL(getpid, r)
{
	r->eax = current->proc->pid;
	return r;
}

KDEF_SYSCALL(execv, r)
{
	elf_header *user_image = (elf_header *) r->ebx;

	char **user_argv = (char **)r->ecx;
	char **user_env = (char **)r->edx;
	
	int i=0, j=0, k;
	if(user_argv)
	{
		while(user_argv[i])
			i++;
		while(user_env[j])
			j++;
	} else {
		i = j = 0;
	}

	char **argv = (char **)kmalloc(sizeof(char *)*i);
	char **env = (char **)kmalloc(sizeof(char *)*j);

	for(k=0; k<i; k++)
	{
		uint32_t size = (strlen(user_argv[k])+1)*sizeof(char);
		argv[k] = (char *)kmalloc(size);
		memcopy(argv[k],user_argv[k], size);
	}
	for(k=0; k<j; k++)
	{
		uint32_t size = (strlen(user_env[k])+1)*sizeof(char);
		env[k] = (char *)kmalloc(size);
		memcopy(env[k],user_env[k], size);
	}


	elf_header *image = (elf_header *)kmalloc(0x1000);
	memcopy(image, user_image, 0x1000);

	elf_t *elf = &current->proc->elf;

	// Clear memory of process here
	
	load_elf(image, elf);
	current->r.eip = elf->entry;
	current->r.cs = SEG_USER_CODE | 0x3;
	current->r.ds = SEG_USER_DATA | 0x3;
	current->r.ss = SEG_USER_DATA | 0x3;
	current->r.useresp = USER_STACK_TOP;
	current->r.ebp = 0;

	current->kernel_thread = &current->r;

	debug("\n Elf %x - %x", elf->start, elf->end);
	// Put arguments and env on the stack here

	uint32_t size = sizeof(char *)*i;
	uint32_t arg_pos = elf->end;
	if(!vmm_page_get(arg_pos + size) & PAGE_PRESENT)
	{
		vmm_page_set(arg_pos + size, vmm_page_val(pmm_alloc_page(), PAGE_PRESENT | PAGE_USER | PAGE_WRITE));
	}
	user_argv = (char **)arg_pos;
	arg_pos += size;
	for (k= 0; k < i; k++)
	{
		uint32_t size = (strlen(argv[k])+1)*sizeof(char);
		if(!vmm_page_get(arg_pos + size) & PAGE_PRESENT)
		{
			vmm_page_set(arg_pos + size, vmm_page_val(pmm_alloc_page(), PAGE_PRESENT | PAGE_USER | PAGE_WRITE));
		}
		user_argv[k] = (char *)arg_pos;
		memcopy(user_argv[k], argv[k], size);
		kfree(argv[k]);
		arg_pos += size;
	}
	kfree(argv);

	vmm_page_set(USER_STACK_TOP - PAGE_SIZE, vmm_page_val(pmm_alloc_page(), PAGE_PRESENT | PAGE_USER | PAGE_WRITE));
	uintptr_t stack = USER_STACK_TOP;

	stack -= sizeof(uint32_t);
	*((uintptr_t *)stack) = (uintptr_t)user_argv; // argv
	stack -= sizeof(uint32_t);
	*((uintptr_t *)stack) = i; // argc
	stack -= sizeof(uint32_t);
	*((uintptr_t *)stack) = 0x0; // Faux return address
	debug("Stack:%x",stack);
	current->r.useresp = stack;


	return &current->r;
}
