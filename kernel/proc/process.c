#include <process.h>
#include <heap.h>
#include <vmm.h>
#include <thread.h>
#include <memory.h>
#include <scheduler.h>
#include <k_debug.h>
#include <lists.h>

uint32_t next_pid = 1;

process_t *init_proc;
list_head_t pid_hash_table[1<<PID_HASH_BITS];

process_t *new_process()
{
	process_t *p = (process_t *)kcalloc(sizeof(process_t));
	p->pid = next_pid++;

	init_list(p->threads);
	init_list(p->waiting);
	init_list(p->pid_list);

	append_to_list(pid_hash_table[pid_hash(p->pid)], p->pid_list);
	debug("\n proc %x at %x", p->pid, p);

	return p;
}

void free_process(process_t *p)
{
	kfree(p);
}

void kill_process()
{
	process_t *p = current->proc;
	list_t *threaditem;
	for_each_in_list(&p->threads, threaditem)
	{
		thread_t *th = list_entry(threaditem, thread_t, proc_threads);
		th->state = TH_STATE_FINISHED;
	}

	process_t *child = p->child;
	while(child)
	{
		child->parent = init_proc;
		child = child->older_sibling;
	}

	p->state = PROC_STATE_FINISHED;

	schedule();
}


void process_make_child(process_t *parent, process_t *child)
{
	child->parent = parent;
	child->older_sibling = parent->child;
	if(child->older_sibling)
		child->older_sibling->younger_sibling = child;
	parent->child = child;
}

process_t *fork_process(process_t *parent, registers_t *r)
{
	process_t *child = new_process();

	process_make_child(parent, child);

	init_list(child->threads);

	thread_t *new_th = alloc_thread();
	uint32_t tid = new_th->tid;
	memcopy(thinfo_from_tcb(new_th), thinfo_from_tcb(current), sizeof(thread_info_t));

	new_th->tid = tid;
	init_list(new_th->proc_threads);
	init_list(new_th->tasks);

	bind_thread(new_th, child);

	new_th->kernel_thread = (registers_t *)((uintptr_t)new_th - ((uintptr_t)current-(uintptr_t)r));

	memcopy(&child->elf, &parent->elf, sizeof(elf_t));
	child->pd = vmm_clone_pd();

	r->eax = child->pid;
	new_th->kernel_thread->eax = 0;

	if((r->cs & 0x3) != 0x3)
	{
		//SHOULD MOVE POINTERS ON STACK HERE
		//move_stack(new_th->kernel_thread, new_th, new_th->kernel_thread -r);
		new_th->kernel_thread->ebp += new_th->kernel_thread -r;
	}

	scheduler_insert(new_th);

	return child;
}

process_t *process_init(void *func)
{

	int i;
	for(i = 0; i < 1<<PID_HASH_BITS; i++)
	{
		init_list(pid_hash_table[i]);
	}

	init_proc = new_process();

	thread_t *th = new_thread(0,0);
	bind_thread(th, init_proc);
	th->r.eip = (uint32_t)func;

	init_proc->pd = vmm_clone_pd();
	th->state = TH_STATE_RUNNING;
	scheduler_insert(th);

	return init_proc;
}

void switch_process(process_t *p)
{
	vmm_set_pd(p->pd);
}

process_t *get_process_by_pid(uint32_t pid)
{
	list_head_t *head = &pid_hash_table[pid_hash(pid)];
	list_t *item;
	for_each_in_list(head, item)
	{
		process_t *proc = list_entry(item, process_t, pid_list);
			if(proc->pid == pid)
			return proc;
	}
	return 0;
}
