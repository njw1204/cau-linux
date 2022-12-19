/* ---------- compare_and_swap_module.c ----------- */

#include <linux/kernel.h>  // Needed by all modules
#include <linux/module.h>  // Needed for KERN_ALERT
#include <linux/init.h>    // Needed for the macros
#include <linux/slab.h>  // for kmalloc
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>

int counter;		// shared resource
volatile int lock = 0;	// lock 1: someone acquired lock, 0: nobody has lock
struct task_struct *thread[4];

static int work_fn(void *data)
{
	int original;

	while (!kthread_should_stop()) {
		// critical section
		// Using lock variable is optional here
		while (__sync_val_compare_and_swap(&lock, 0, 1));
		original = __sync_val_compare_and_swap(&counter, counter, counter + 1);
		__sync_val_compare_and_swap(&lock, 1, 0);
		// end of the critical section
		printk(KERN_INFO "pid[%u] %s: counter: %d\n", 
				current->pid, __func__, original);
		msleep(500);
	}

	do_exit(0);
}

int __init compare_and_swap_module_init(void)
{	
	int i;

	printk(KERN_INFO "%s: Entering Compare and swap Module!\n", __func__);

	for (i = 0; i < 4; i++)
		thread[i] = kthread_run(work_fn, NULL, "compare_and_swap_function");

	return 0;
}

void __exit compare_and_swap_module_cleanup(void)
{	
	int i;
	
	for (i = 0; i < 4; i++)
		kthread_stop(thread[i]);
	
	printk(KERN_INFO "%s: Exiting Compare and Swap Module!\n", __func__);
}

module_init(compare_and_swap_module_init);
module_exit(compare_and_swap_module_cleanup);
MODULE_LICENSE("GPL");
