/*
 * Test linux module enviroment
 */

#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");

static __init int hello_init(void)
{
	printk(KERN_INFO "Hello linux module\n");
	return 0;
}

static __exit void hello_exit(void)
{
	printk(KERN_INFO "Goodbye linux module\n");
}

module_init(hello_init);
module_exit(hello_exit);
