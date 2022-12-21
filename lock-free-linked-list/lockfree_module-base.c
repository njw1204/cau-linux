/* ---------- lockfree_module-base.c ----------- */
#include "linked_list_impl.h"
#include <linux/kthread.h>
#include <linux/delay.h>
#include "lockfree_list.h"

struct animal *head;
struct task_struct *thread[4], *gc_thread;

int params[4] = {1, 2, 3, 4};

static int work_fn(void *data)
{
	int range_bound[2];
	int err, thread_id = *(int*) data;

	set_iter_range(thread_id, range_bound);
	add_to_list(thread_id, range_bound);
	err = search_list(thread_id, range_bound);
	if (!err)
		delete_from_list(thread_id, range_bound);

	while (!kthread_should_stop()) {
		msleep(500);
	}
	printk(KERN_INFO "thread #%d stopped!\n", thread_id);

	return 0;
}

int empty_garbage_list(void)
{
	struct cat *cur;
	struct list_head *entry, *iter = &head->gc_entry;
	unsigned int counter = 0;
	
	/* iterate garbage list, 
		remove each entry from every list and free it */
	while ((entry = iter) != NULL) {
		iter = iter->next;

		if (!entry->prev->next)
			continue;
		
		cur = list_entry(entry, struct cat, gc_entry);
		
		if (!atomic_read(&cur->removed))
			continue;
		
		gc_list_del(&cur->entry, &head->entry);
		gc_list_del(&cur->gc_entry, &head->gc_entry);
		kfree(cur);
		counter++;
	}

	printk(KERN_INFO "%s: freed %u cats\n", __func__, counter);

	return 0;
}

void destroy_list(void)
{
	struct cat *cur;
	struct list_head *entry, *iter = &head->entry;
	
	/* iterate the list, 
		remove each entry from every list and free it */
	while ((entry = iter) != NULL) {
		iter = iter->next;

		if (!entry->prev->next)
			continue;
		
		cur = list_entry(entry, struct cat, entry);
		
		gc_list_del(&cur->entry, &head->entry);
		__sync_fetch_and_sub(&head->total, 1);
		kfree(cur);
	}
}

int __init lockfree_module_init(void)
{
	printk("%s: Entering Lock-free Module!\n", __func__);
	
	/* initialize struct animal here */
	head = kmalloc(sizeof(struct animal), GFP_KERNEL);

	/* initialize animal's list head and gc_list head here */
	INIT_LF_LIST_HEAD(&head->entry);
	INIT_LF_LIST_HEAD(&head->gc_entry);

	/* start each thread here */
	int i;

	for (i = 0; i < 4; i++) {
		thread[i] = kthread_run(work_fn, &params[i], "kthread_work_fn");
	}

	return 0;
}

extern unsigned long long add_to_list_time, add_to_list_count;
extern unsigned long long search_list_time, search_list_count;
extern unsigned long long delete_list_time, delete_list_count;

void __exit lockfree_module_cleanup(void)
{		
	/* print calclock result here */
	printk("%s: Lock-free linked list insert time: %llu ns, count: %llu\n", 
		__func__, add_to_list_time, add_to_list_count);
	printk("%s: Lock-free linked list search time: %llu ns, count: %llu\n", 
		__func__, search_list_time, search_list_count);
	printk("%s: Lock-free linked list delete time: %llu ns, count: %llu\n", 
		__func__, delete_list_time, delete_list_count);

	/* stop every thread here */
	int i;

	for (i = 0; i < 4; i++) {
		kthread_stop(thread[i]);
	}

	empty_garbage_list();
	printk("After gc: total %d cats\n", head->total);

	destroy_list();
	printk("After destroyed list: total %d cats\n", head->total);
	kfree(head);

	printk("%s: Exiting Lock-free Module!\n", __func__);
}

module_init(lockfree_module_init);
module_exit(lockfree_module_cleanup);
MODULE_LICENSE("GPL");
