#ifndef _LINKED_LIST_IMPLEMENTS_H
#define _LINKED_LIST_IMPLEMENTS_H

#include <linux/kernel.h>  // Needed by all modules
#include <linux/module.h>  // Needed for KERN_ALERT
#include <linux/init.h>    // Needed for the macros
#include <linux/slab.h>  // for kmalloc

#define SIZE 10000000

struct animal {
	int total;
	struct list_head entry;
	atomic_t removed;	/* for delete */
	struct list_head gc_entry;
};

struct cat {
	int var;
	struct list_head entry;
	atomic_t removed;	/* for delete */
	struct list_head gc_entry;
};

static inline void set_iter_range(int thread_num, int bound[])
{
    switch (thread_num) {
        case 1:
            bound[0] = 0;
            bound[1] = SIZE / 4 - 1;
            break;
        case 2:
            bound[0] = SIZE / 4;
            bound[1] = SIZE / 2 - 1;
            break;
        case 3:
            bound[0] = SIZE / 2;
            bound[1] = SIZE * 3/4 -1;
            break;
        case 4:
            bound[0] = SIZE * 3/4;
            bound[1] = SIZE - 1;
            break;
    } 
}

static inline int select_target_index(int range_bound[])
{
	int length = range_bound[1] - range_bound[0] + 1;

	return length / 2 + range_bound[0];
}

void add_to_list(int thread_id, int range_bound[]);
int search_list(int thread_num, int range_bound[]);
void delete_from_list(int thread_id, int range_bound[]);

#endif /* _LINKED_LIST_EXAMPLE_H */
