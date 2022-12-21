/* ---------- fetch_and_add_module.c ----------- */

#include <linux/kernel.h>  // Needed by all modules
#include <linux/module.h>  // Needed for KERN_ALERT
#include <linux/init.h>    // Needed for the macros
#include <linux/slab.h>  // for kmalloc
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>

int counter;	// shared resource
struct task_struct *thread[4];

static int work_fn(void *data)
{
	int original;

	while (!kthread_should_stop()) {
		// critical section
		original = __sync_fetch_and_add(&counter, 1);
		// end of the critical section
		printk(KERN_INFO "pid[%u] %s: counter: %d\n", 
				current->pid, __func__, original);
		msleep(500);
	}

	do_exit(0);
}

int __init fetch_and_add_module_init(void)
{	
	int i;

	printk(KERN_INFO "%s: Entering Fetch and Add Module!\n", __func__);

	for (i = 0; i < 4; i++)
		thread[i] = kthread_run(work_fn, NULL, "fetch_and_add_function");

	return 0;
}

void __exit fetch_and_add_module_cleanup(void)
{	
	int i;
	
	for (i = 0; i < 4; i++)
		kthread_stop(thread[i]);
	
	printk(KERN_INFO "%s: Exiting Fetch and Add Module!\n", __func__);
}

module_init(fetch_and_add_module_init);
module_exit(fetch_and_add_module_cleanup);
MODULE_LICENSE("GPL");
