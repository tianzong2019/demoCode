#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/dma-buf.h>


static int dmisc_open(struct inode *inode, struct file *filp)
{
    printk("%s %d\n",__FUNCTION__,__LINE__);
    return 0;
}
static int dmisc_release(struct inode *inode, struct file *filp)
{
    printk("%s %d\n",__FUNCTION__,__LINE__);
    return 0;
}
static ssize_t dmisc_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	printk("%s %d\n",__FUNCTION__,__LINE__);
    return 1;
}

static ssize_t dmisc_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	int ret = 0, ionfd = 0;
	ret = copy_from_user(&ionfd, buf, count);
	printk("%s %d, ionfd 0x%x\n",__FUNCTION__,__LINE__, ionfd);
	//
	struct dma_buf *dbuf = dma_buf_get(ionfd);
	printk("%s %d, dbuf 0x%x, size %d\n",__FUNCTION__,__LINE__, dbuf, dbuf->size);
	//
	dma_buf_begin_cpu_access(dbuf, DMA_FROM_DEVICE);
	//
	void *vaddr = dma_buf_kmap(dbuf, 0);
	if (IS_ERR(vaddr)) {
		printk("%s %d, dma_buf_kmap error\n",__FUNCTION__,__LINE__);
		goto exit;
	}
	printk("%s %d, *vaddr %p %d\n",__FUNCTION__,__LINE__, vaddr, *((int *)vaddr));
	dma_buf_kunmap(dbuf, 0, vaddr);
	//
exit:
	dma_buf_end_cpu_access(dbuf, DMA_FROM_DEVICE);
	//
	dma_buf_put(dbuf);
    return 1;
}
static long dmisc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    printk("%s %d\n",__FUNCTION__,__LINE__);
    return 1;
}

static const struct file_operations mdrv_fops =
{
    .owner = THIS_MODULE,
    .open = dmisc_open,
    .read = dmisc_read,
    .write = dmisc_write,
    .unlocked_ioctl = dmisc_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = dmisc_ioctl,
#endif
    .release = dmisc_release
};

static struct miscdevice mdrv_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "miscdev",
	.fops = &mdrv_fops,
};
static int __init mdrv_init(void)
{
	printk("%s %s %d\n",__FILE__,__FUNCTION__,__LINE__);
	misc_register(&mdrv_dev);
	return 0;
}
static void __exit mdrv_exit(void)
{
	printk("%s %s %d\n",__FILE__,__FUNCTION__,__LINE__);
	misc_deregister(&mdrv_dev);
}
module_init(mdrv_init);
module_exit(mdrv_exit);
MODULE_INFO(intree, "Y");
MODULE_LICENSE("GPL");
