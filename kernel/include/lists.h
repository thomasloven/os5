#pragma once
// This should move to the library sooner or later (rather sooner)
#ifndef __ASSEMBLER__

typedef struct list_struct
{
	struct list_struct *prev;
	struct list_struct *next;
} list_t;

typedef list_t list_head_t;

#define for_each_in_list(head, item) \
	for((item) = (head)->next; (item) != (head); (item) = (item)->next)
#define offsetof(type, member) \
	((size_t) ( (char *)&((type *)0)->member - (char *)0))
#define list_entry(ptr, type, member) ({\
	const __typeof__( ((type *)0)->member ) *__mptr = (ptr); \
	(type *)( (char *)__mptr - offsetof(type,member));})

#define init_list(head) \
	head.next = &(head);\
	head.prev = &(head)

#define append_to_list(head, item) \
	(item).prev = (head).prev; \
	(item).next = &(head); \
	(item).prev->next = &(item); \
	(item).next->prev = &(item);

#define remove_from_list(item) \
	(item).prev->next = (item).next; \
	(item).next->prev = (item).prev; \
	(item).next = &(item); \
	(item).prev = &(item)

#define list_empty(head) \
	(((head).next == &(head))&&((head).prev == &(head)))

#endif
