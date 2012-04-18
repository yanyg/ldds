/*
 * Reference drivers/block/brd.c
 */

#include <linux/genhd.h>
#include <linux/blkdev.h>

/* module params */
static int rd_major;
static int rd_max_part;
static int rd_size;
static int rd_nrd;

module_param(rd_major, int, S_IRUGO);
MODULE_PARM_DESC(rd_major, "Requested major device number");
module_param(rd_max_part, int, S_IRUGO);
MODULE_PARM_DESC(rd_max_part, "Maximum number of partitions per ramdisk");
module_param(rd_size, int, S_IRUGO);
MODULE_PARM_DESC(rd_size, "Size of ramdisk in kilobytes");
module_param(rd_nrd, int, S_IRUGO);
MODULE_PARM_DESC(rd_nrd, "Number of ramdisks when module loaded");

static int part_shift;

int params_init(void)
{
	if (0 == rd_max_part)
		rd_max_part = 16;

	/* adjusting part */
	part_shift = fls(rd_max_part);
	max_part = (1UL << part_shift) - 1;
	if ((1UL << part_shift) > DISK_MAX_PARTS)
		return -EINVAL;

	if (0 == rd_nrd)
		rd_nrd = 8;

	if (rd_nrd > (1UL << (MINORBITS - part_shift)))
		return -EINVAL;

	if (0 == rd_size)
		rd_size = 1024 * 10;	/* 1024*10KB = 10MB */

	return 0;
}

static __init int rd_init(void)
{
	int i, err;

	struct rd_device *rd, *next;

	if ((err = params_init()) < 0) {
		printk(KERN_ERR "params_init failed");
		return err;
	}

	if ((err = register_blkdev(rd_major, "rd")) < 0) {
		printk(KERN_ERR "register_blkdev failed (major=%d)", rd_major);
		return err;
	}
	if (rd_major == 0)
		rd_major = err;	/* allocated major dynamically */

	for (i = 0; i < rd_nrd; ++i) {
		rd = rd_alloc(i);
		if (!rd)
			goto err_out_alloc;

		list_add_tail(&rd->rd_list, &rd_devices);
	}

	list_for_each_entry(rd, &rd_devices, rd_list)
		add_disk(rd->rd_disk);

	blk_register_region(MKDEV(rd_major, 0), range, 
			    THIS_MODULE, rd_probe, NULL, NULL);

	printk(KERN_INFO "block-rd: rd loaded");
	return 0;

err_out_alloc:
	list_for_each_entry_safe(rd, next, &rd_devices, rd_list) {
		list_del(rd->rd_list);
		rd_free(rd);
	}
	unregister_blkdev(rd_major, "rd");
	return -ENOMEM;
}

static __exit void rd_exit(void)
{
	unregister_blkdev(rd_major, "rd");
}

module_init(rd_init);
module_exit(rd_exit);

MODULE_LICENSE("GPLv2");
