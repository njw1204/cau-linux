#include "linked_list_impl.h"
#include <linux/kthread.h>
#include <linux/delay.h>

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

void destroy_list(void)
{
	struct cat *cur;
	struct list_head *entry, *iter = &head->entry;
	
	/* iterate the list, 
		remove each entry from every list and free it */
	list_for_each_safe(iter, entry, &head->entry) {
		cur = list_entry(iter, struct cat, entry);
		list_del(iter);
		head->total--;
		kfree(cur);
	}
}

int __init spinlock_module_init(void)
{
	printk("%s: Entering Spinlock Module!\n", __func__);
	
	/* initialize struct animal here */
	head = kmalloc(sizeof(struct animal), GFP_KERNEL);
	INIT_LIST_HEAD(&head->entry);

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

void __exit spinlock_module_cleanup(void)
{		
	/* print calclock result here */
	printk("%s: Spinlock linked list insert time: %llu ns, count: %llu\n", 
		__func__, add_to_list_time, add_to_list_count);
	printk("%s: Spinlock linked list search time: %llu ns, count: %llu\n", 
		__func__, search_list_time, search_list_count);
	printk("%s: Spinlock linked list delete time: %llu ns, count: %llu\n", 
		__func__, delete_list_time, delete_list_count);

	/* stop every thread here */
	int i;

	for (i = 0; i < 4; i++) {
		kthread_stop(thread[i]);
	}

	destroy_list();
	printk("After destroyed list: total %d cats\n", head->total);
	kfree(head);

	printk("%s: Exiting Spinlock Module!\n", __func__);
}

module_init(spinlock_module_init);
module_exit(spinlock_module_cleanup);
MODULE_LICENSE("GPL");
