#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("yanyg <yanyg02@gmail.com>");
MODULE_VERSION("v1.0");

static int __init your_init(void)
{
	return 0;
}

static void __exit your_exit(void)
{

}

module_init(your_init);
module_exit(your_exit);
