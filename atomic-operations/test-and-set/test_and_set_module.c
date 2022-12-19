/* ---------- test_and_set_module.c ----------- */

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
		// while (...)  <--- acquire lock here
		while (__sync_lock_test_and_set(&lock, 1));
		original = __sync_lock_test_and_set(&counter, counter + 1);
		// release lock here
		__sync_lock_test_and_set(&lock, 0);
		// end of the critical section
		printk(KERN_INFO "pid[%u] %s: counter: %d\n", 
				current->pid, __func__, original);
		msleep(500);
	}

	do_exit(0);
}

int __init test_and_set_module_init(void)
{	
	int i;

	printk(KERN_INFO "%s: Entering Test and Set Module!\n", __func__);

	for (i = 0; i < 4; i++)
		thread[i] = kthread_run(work_fn, NULL, "test_and_set_function");

	return 0;
}

void __exit test_and_set_module_cleanup(void)
{	
	int i;
	
	for (i = 0; i < 4; i++)
		kthread_stop(thread[i]);
	
	printk(KERN_INFO "%s: Exiting Test and Set Module!\n", __func__);
}

module_init(test_and_set_module_init);
module_exit(test_and_set_module_cleanup);
MODULE_LICENSE("GPL");
