#include "../linked_list_impl.h"
#include <linux/list.h>
#include <linux/spinlock.h>
#include "../calclock.h"

// define your spinlock here
DEFINE_SPINLOCK(list_spinlock);

// initialize your list here
LIST_HEAD(my_list);

unsigned long long add_to_list_time, add_to_list_count;

/**
 * add_to_list() - add new entries to list.
 * @thread_id: number of current thread who is calling this function.
 * @range_bound: 
 * 	range_bound[0]: lower boundary for this thread.
 * 	range_bound[1]: upper boundary for this thread.
 *
 * Return: The first entry that was inserted with the current thread.
 */
void *add_to_list(int thread_id, int range_bound[])
{
	struct timespec localclock[2];
    struct my_node *new, *first = NULL;
	int i;

	printk(KERN_INFO "thread #%d range: %d ~ %d\n", 
			thread_id, range_bound[0], range_bound[1]);
	
	// put your code here
	getrawmonotonic(&localclock[0]);
	
	for (i = range_bound[0]; i <= range_bound[1]; i++) {
		new = kmalloc(sizeof(struct my_node), GFP_KERNEL);
		new->data = i;

		if (first == NULL) {
			first = new;
		}
		
		spin_lock(&list_spinlock);
		list_add_tail(&new->list, &my_list);
		spin_unlock(&list_spinlock);
		getrawmonotonic(&localclock[1]);
		calclock(localclock, &add_to_list_time, &add_to_list_count);
		getrawmonotonic(&localclock[0]);
	}

	return first;
}

unsigned long long search_list_time, search_list_count;

/**
 * search_list() - iterate over the list.
 * @thread_id: number of current thread who is calling this function.
 * @data: pointer to the first entry with current thread.
 * @range_bound: 
 * 	range_bound[0]: lower boundary for this thread.
 * 	range_bound[1]: upper boundary for this thread.
 *
 * Return: 0 on success.
 */
int search_list(int thread_id, void *data, int range_bound[])
{
	struct timespec localclock[2];
	/* This will point on the actual data structures during the iteration */
	struct my_node *cur = (struct my_node *) data, *tmp;

	// put your code here
	printk(KERN_INFO "thread #%d searched range: %d ~ %d\n", 
			thread_id, range_bound[0], range_bound[1]);

	getrawmonotonic(&localclock[0]);
	spin_lock(&list_spinlock);

	for (tmp = list_next_entry(cur, list); &cur->list != &my_list;
			cur = tmp, tmp = list_next_entry(tmp, list)) {
		if (cur->data >= range_bound[0] && cur->data <= range_bound[1]) {
			getrawmonotonic(&localclock[1]);
			calclock(localclock, &search_list_time, &search_list_count);
			getrawmonotonic(&localclock[0]);
		}
	}

	spin_unlock(&list_spinlock);
	
	return 0;
}

unsigned long long delete_list_time, delete_list_count;

/**
 * delete_from_list() - delete entries from the list.
 * @thread_id: number of current thread who is calling this function.
 * @range_bound: 
 * 	range_bound[0]: lower boundary for this thread.
 * 	range_bound[1]: upper boundary for this thread.
 *
 * Return: 0 on success.
 */
int delete_from_list(int thread_id, int range_bound[])
{
	struct my_node *cur, *tmp;
	struct timespec localclock[2];

	// put your code here
	printk(KERN_INFO "thread #%d deleted range: %d ~ %d\n", 
			thread_id, range_bound[0], range_bound[1]);

	getrawmonotonic(&localclock[0]);
	spin_lock(&list_spinlock);

	list_for_each_entry_safe(cur, tmp, &my_list, list) {
		if (cur->data >= range_bound[0] && cur->data <= range_bound[1]) {
			list_del(&cur->list);
			kfree(cur);
			getrawmonotonic(&localclock[1]);
			calclock(localclock, &delete_list_time, &delete_list_count);
			getrawmonotonic(&localclock[0]);
		}
	}

	spin_unlock(&list_spinlock);
	
	return 0;
}
