#include <process.h>
#include <heap.h>
#include <vmm.h>
#include <thread.h>
#include <memory.h>

uint32_t next_pid = 1;

process_t *new_process()
{
	process_t *p = (process_t *)kcalloc(sizeof(process_t));
	p->pid = next_pid++;

	return p;
}

process_t *fork_process(thread_t *th)
{
	process_t *parent = current->proc;
	process_t *child = new_process();

	child->parent = parent;
	child->older_sibling = parent->child;
	if(child->older_sibling) child->older_sibling->younger_sibling = child;
	parent->child = child;

	init_list(child->threads);

	if(th)
	{
		// Clone thread th as main thread
	} else {
		// Clone first thread as main thread
	}

	memcopy(&child->elf, &parent->elf, sizeof(elf_t));
	child->pd = vmm_clone_pd();

	return child;
}

process_t *process_init(thread_t *th)
{
	process_t *init = new_process();

	init_list(init->threads);

	append_to_list(init->threads, th->proc_threads);

	init->pd = vmm_clone_pd();

	th->proc = init;

	return init;
}

void switch_process(process_t *p)
{
	vmm_set_pd(p->pd);
}

