#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

static int __init najongwoo_module_init(void) {
    printk("simple module\n");
    printk("made by najongwoo\n");
    return 0;
}

static void __exit najongwoo_module_cleanup(void) {
    printk("bye bye\n");
}

module_init(najongwoo_module_init);
module_exit(najongwoo_module_cleanup);
MODULE_LICENSE("GPL");
