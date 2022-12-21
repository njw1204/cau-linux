/* ---------- rwsem_module-base.c ----------- */
#include "../linked_list_impl.h"
#include <linux/kthread.h>
#include <linux/delay.h>

struct task_struct *thread[4];

int params[4] = {1, 2, 3, 4};

static int work_fn(void *data)
{
	int range_bound[2];
	int thread_id = *(int*) data;

	set_iter_range(thread_id, range_bound);
	void *ret = add_to_list(thread_id, range_bound);
	search_list(thread_id, ret, range_bound);
	delete_from_list(thread_id, range_bound);

	while(!kthread_should_stop()) {
		msleep(500);
	}
	printk(KERN_INFO "thread #%d stopped!\n", thread_id);

	return 0;
}

int __init rwsem_module_init(void)
{	
	int i;

	printk("%s: Entering RW Semaphore Module!\n", __func__);
	
	for (i = 0; i < 4; i++)
		thread[i] = kthread_run(work_fn, &params[i], "kthread_work_fn");

	return 0;
}

extern unsigned long long add_to_list_time, add_to_list_count;
extern unsigned long long search_list_time, search_list_count;
extern unsigned long long delete_list_time, delete_list_count;

void __exit rwsem_module_cleanup(void)
{	
	int i;

	printk("%s: RW Semaphore linked list insert time: %llu ns, count: %llu\n", 
		__func__, add_to_list_time, add_to_list_count);
	printk("%s: RW Semaphore linked list search time: %llu ns, count: %llu\n", 
		__func__, search_list_time, search_list_count);
	printk("%s: RW Semaphore linked list delete time: %llu ns, count: %llu\n", 
		__func__, delete_list_time, delete_list_count);
	
	for (i = 0; i < 4; i++)
		kthread_stop(thread[i]);
	
	printk("%s: Exiting RW Semaphore Module!\n", __func__);
}

module_init(rwsem_module_init);
module_exit(rwsem_module_cleanup);
MODULE_LICENSE("GPL");
