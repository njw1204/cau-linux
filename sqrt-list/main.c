#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include "linux/list.h"
#include "linux/sqrt_list.h"
#include "calclock.h"

static int __init sqrt_list_module_init(void)
{
  printk("SQRT List Module Init\n");
  return 0;
}

static void __exit sqrt_list_module_exit(void)
{
  printk("SQRT List Module Exit\n");
}

module_init(sqrt_list_module_init);
module_exit(sqrt_list_module_exit);
MODULE_LICENSE("GPL");
