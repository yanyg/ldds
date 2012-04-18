/*
 * block-simple.c
 */

#include <linux/blkdev.h>
#include <linux/genhd.h>

static struct gendisk *bsd;	/* block simple disk */
static struct request_queue *bsd_queue;

static dev_t bsd_major = 0;

static DEFINE_SPINLOCK(bsd_lock);

static int bsd_size = 256 * 1024;
static int bsd_sector_size = 512;

static int bsd_ioctl(struct block_device *dev, fmode_t mode,
		     unsigned cmd, unsigned long arg)
{
	switch(cmd) {
	case 0xAA00:
		return 0xffee;
	default:
		return -EINVAL;
	}
}

static struct block_device_operations bsd_fops = {
	.owner = THIS_MODULE,
	.ioctl = bsd_ioctl,
};

static void bsd_request(struct request_queue *rq)
{
	struct request *q;
	int i;

	while ((q = blk_fetch_request(rq))) {
		if (blk_fs_request(q)) {
			switch (rq_data_dir(q)) {
			case READ:
				for (i = 0; i < blk_rq_sectors(q) * 512; ++i) {
					q->buffer[i] = i%10 + '0';
				}
				break;
			case WRITE:
				break;
			}
		}
		blk_end_request(q, 0, blk_rq_sectors(q)*512);
	}
}

static __init int bsd_init(void)
{
	int err = -EIO;

	if ((err = register_blkdev(bsd_major, "bsd")) <= 0)
		return err;
	bsd_major = err;

	if (!(bsd_queue = blk_init_queue(bsd_request, &bsd_lock)))
		goto err_out_queue;

	blk_queue_logical_block_size(bsd_queue, bsd_sector_size);
	blk_queue_max_sectors(bsd_queue, bsd_size/bsd_sector_size);

	if (!(bsd = alloc_disk(1)))
		goto err_out_disk;

	bsd->fops = &bsd_fops;
	set_capacity(bsd, bsd_size);
	bsd->queue = bsd_queue;
	bsd->major = bsd_major;
	bsd->first_minor = 0;
	sprintf(bsd->disk_name, "bsd");

	add_disk(bsd);

	return 0;

err_out_disk:
	blk_cleanup_queue(bsd_queue);

err_out_queue:
	unregister_blkdev(bsd_major, "bsd");

return err;
}

static __exit void bsd_exit(void)
{
	del_gendisk(bsd);
	put_disk(bsd);
	blk_cleanup_queue(bsd_queue);
	unregister_blkdev(bsd_major, "bsd");
}

module_init(bsd_init);
module_exit(bsd_exit);

MODULE_LICENSE("GPL");
