#include "linked_list_impl.h"
#include <asm/errno.h>
#include "lockfree_list.h"
#include "calclock.h"

extern struct animal *head;

unsigned long long add_to_list_time, add_to_list_count;

/**
 * add_to_list() - add new entries to list.
 * @thread_id: number of current thread who is calling this function.
 * @range_bound: 
 * 	range_bound[0]: lower boundary for this thread.
 * 	range_bound[1]: upper boundary for this thread.
 */
void add_to_list(int thread_id, int range_bound[])
{
	struct timespec localclock[2];
    struct cat *new, *first = NULL;
	int i;

	for (i = range_bound[0]; i < range_bound[1] + 1; i++) {
		getrawmonotonic(&localclock[0]);
		/* initialize new cat here */
		new = kmalloc(sizeof(struct cat), GFP_KERNEL);
		new->var = i;

		if (!first)
			first = new;
		/* initialize cat's list head and gc_list head here */
		INIT_LF_LIST_HEAD(&new->entry);
		INIT_LF_LIST_HEAD(&new->gc_entry);

		/* add new cat into animal's list */
		atomic_set(&new->removed, 0);
		lf_list_add_tail(&new->entry, &head->entry);
		__sync_fetch_and_add(&head->total, 1);

		getrawmonotonic(&localclock[1]);
		calclock(localclock, &add_to_list_time, &add_to_list_count);
	}

	printk(KERN_INFO "thread #%d: inserted cat #%d-%d to the list, total: %u cats\n", 
			thread_id, first->var, new->var, head->total);
}

unsigned long long search_list_time, search_list_count;

/**
 * search_list() - iterate over the list.
 * @thread_id: number of current thread who is calling this function.
 * @range_bound: 
 * 	range_bound[0]: lower boundary for this thread.
 * 	range_bound[1]: upper boundary for this thread.
 *
 * Return: 0 on success, ENODATA when no matching entry in the list.
 */
int search_list(int thread_id, int range_bound[])
{
	struct list_head *entry, *iter = &head->entry;
	/* This will point on the actual data structures during the iteration */
	struct cat *cur;
	struct timespec localclock[2];
	int target_idx = select_target_index(range_bound);

	/* iterate over the list, skip removed entry, 
		search cat with target index */
	while((entry = iter) != NULL){
		getrawmonotonic(&localclock[0]);

		if (__sync_val_compare_and_swap(&iter, entry, iter->next) != entry) {
			continue;
		}
		
		if (!entry->prev->next) {
			continue;
		}
		
		cur = list_entry(entry, struct cat, entry);

		if (atomic_read(&cur->removed)) {
			continue;
		}
		
		if (cur->var == target_idx) {
			printk(KERN_INFO "thread #%d: found cat #%d\n", thread_id, cur->var);
			return 0;
		}

		getrawmonotonic(&localclock[1]);
		calclock(localclock, &search_list_time, &search_list_count);
	}

	return -ENODATA;
}

/**
 * add_to_garbage_list() - mark entry in the list as deleted and add to garbage list.
 * @thread_id: number of current thread who is calling this function.
 * @data: entry to add to the garbage list
 */
static void add_to_garbage_list(int thread_id, void *data)
{
	struct cat *target = (struct cat *) data;
	
	/* set cat as deleted */
	atomic_set(&target->removed, 1);

	/* add to garbage list */
	lf_list_add_tail(&target->gc_entry, &head->gc_entry);
	
	/* decrease total cats in struct animal */
	__sync_fetch_and_sub(&head->total, 1);
}

unsigned long long delete_list_time, delete_list_count;

void delete_from_list(int thread_id, int range_bound[])
{
	int start = -1, end = -1;
	struct timespec localclock[2];
	struct list_head *entry, *iter = &head->entry;
	/* This will point on the actual data structures during the iteration */
	struct cat *cur;
	int target_idx = select_target_index(range_bound);

	/* iterate over the list, skip removed entry */
	while((entry = iter) != NULL){
		getrawmonotonic(&localclock[0]);

		if (__sync_val_compare_and_swap(&iter, entry, iter->next) != entry) {
			continue;
		}
		
		if (!entry->prev->next) {
			continue;
		}
		
		cur = list_entry(entry, struct cat, entry);

		if (atomic_read(&cur->removed)) {
			continue;
		}
		
		/* add cat into garbage list if its id is within target range */
		int pos = cur->var;

		if (pos >= target_idx && pos <= range_bound[1]) {
			if (start == -1) {
				start = pos;
			}

			end = pos;
			add_to_garbage_list(thread_id, cur);
		}

		getrawmonotonic(&localclock[1]);
		calclock(localclock, &delete_list_time, &delete_list_count);
	}

	printk(KERN_INFO "thread #%d: marked cat #%d-%d as deleted, total: %d cats\n", 
			thread_id, start, end, head->total);
}
