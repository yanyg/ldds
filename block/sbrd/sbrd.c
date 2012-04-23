/*
 * simple block ramdisk
 */

#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/radix-tree.h>

struct sbrd_device {
	int sbrd_number;

	struct request_queue	*sbrd_queue;
	struct gendisk		*sbrd_disk;
	struct list_head	sbrd_list;

	spinlock_t		sbrd_lock;
	struct radix_tree_root	*sbrd_pages;
};

/* module params */
static int sbrd_major;
static int sbrd_max_part;
static int sbrd_size;
static int sbrd_nrd;

module_param(sbrd_major, int, S_IRUGO);
MODULE_PARM_DESC(sbrd_major, "Requested major device number");
module_param(sbrd_max_part, int, S_IRUGO);
MODULE_PARM_DESC(sbrd_max_part, "Maximum number of partitions per ramdisk");
module_param(sbrd_size, int, S_IRUGO);
MODULE_PARM_DESC(sbrd_size, "Size of ramdisk in kilobytes");
module_param(sbrd_nrd, int, S_IRUGO);
MODULE_PARM_DESC(sbrd_nrd, "Number of ramdisks when module loaded");

static int part_shift;

int params_init(void)
{
	if (0 == sbrd_max_part)
		sbrd_max_part = 16;

	/* adjusting part */
	part_shift = fls(sbrd_max_part);
	max_part = (1UL << part_shift) - 1;
	if ((1UL << part_shift) > DISK_MAX_PARTS)
		return -EINVAL;

	if (0 == sbrd_nrd)
		sbrd_nrd = 8;

	if (sbrd_nrd > (1UL << (MINORBITS - part_shift)))
		return -EINVAL;

	if (0 == sbrd_size)
		sbrd_size = 1024 * 10;	/* 1024*10KB = 10MB */

	return 0;
}

static const struct block_device_operations sbrd_fops = {
	.owner =		THIS_MODULE,
	.ioctl =		sbrd_ioctl,
	.direct_access =	sbrd_direct_access,
};

static __init int sbrd_init(void)
{
	int i, err;

	struct sbrd_device *rd, *next;

	if ((err = params_init()) < 0) {
		printk(KERN_ERR "params_init failed");
		return err;
	}

	if ((err = register_blkdev(sbrd_major, "rd")) < 0) {
		printk(KERN_ERR "register_blkdev failed (major=%d)", sbrd_major);
		return err;
	}
	if (sbrd_major == 0)
		sbrd_major = err;	/* allocated major dynamically */

	for (i = 0; i < sbrd_nrd; ++i) {
		rd = sbrd_alloc(i);
		if (!rd)
			goto err_out_alloc;

		list_add_tail(&rd->sbrd_list, &sbrd_devices);
	}

	list_for_each_entry(rd, &sbrd_devices, sbrd_list)
		add_disk(rd->sbrd_disk);

	blk_register_region(MKDEV(sbrd_major, 0), range, 
			    THIS_MODULE, sbrd_probe, NULL, NULL);

	printk(KERN_INFO "block-rd: rd loaded");
	return 0;

err_out_alloc:
	list_for_each_entry_safe(rd, next, &sbrd_devices, sbrd_list) {
		list_del(rd->sbrd_list);
		sbrd_free(rd);
	}
	unregister_blkdev(sbrd_major, "rd");
	return -ENOMEM;
}

static __exit void sbrd_exit(void)
{
	unregister_blkdev(sbrd_major, "rd");
}

module_init(sbrd_init);
module_exit(sbrd_exit);

MODULE_LICENSE("GPL v2");
