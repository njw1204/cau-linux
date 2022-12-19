#include <linux/syscalls.h>

SYSCALL_DEFINE0(jongwoocall)
{
	printk("System Call with Na Jongwoo\n");
	return 0;
}
