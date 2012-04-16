#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/pci.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/sysfs.h>
#include <linux/device.h>

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("yanyg <yanyg02@gmail.com>");
MODULE_VERSION("v1.0");

struct input_dev *vms_input_dev;
struct platform_device *vms_dev;

static ssize_t
write_vms(struct device *dev,
	  struct device_attribute *attr,
	  const char *buf,
	  size_t n)
{
	int x, y;

	sscanf(buf, "%d%d", &x, &y);

	input_report_rel(vms_input_dev, REL_X, x);
	input_report_rel(vms_input_dev, REL_Y, y);
	input_sync(vms_input_dev);

	return n;
}

DEVICE_ATTR(coordinates, 0644, NULL, write_vms);

struct attributes *vms_attrs[] = {
	&dev_attr_coordinates.attr,
	NULL
};

struct attribute_group vms_attr_group = {
	.attrs = vms_attrs,
};

int __init vms_init(void)
{
	vms_dev = platform_device_register_simple("vms", -1, NULL, 0);
	if (IS_ERR(vms_dev)) {
		printk(KERN_ERR "platform_device_register_simple fail: %x",
		       PTR_ERR(vms_dev));
		return -1;
	}

	sysfs_create_group(&vms_dev->dev.kobj, &vms_attr_group);

	vms_input_dev = input_allocate_device();
	if (!vms_input_dev) {
		printk(KERN_ERR "input_allocate_device fail");
		return -1;
	}

	set_bit(EV_REL, vms_input_dev->evbit);
	set_bit(REL_X, vms_input_dev->relbit);
	set_bit(REL_Y, vms_input_dev->relbit);

	input_register_device(vms_input_dev);
	printk(KERN_INFO "vms_init success");

	return 0;
}

void __exit vms_exit(void)
{
	input_unregister_device(vms_input_dev);
	sysfs_remove_group(&vms_dev->dev.kobj, &vms_attr_group);
	platform_device_unregister(vms_dev);
}

module_init(vms_init);
module_exit(vms_exit);
