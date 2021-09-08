#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#include "/home/albter/codespace/qemu.arm/linux-4.11/drivers/staging/android/ion/ion.h"

static int dmisc1_open(struct inode *inode, struct file *filp)
{
    printk("%s %d\n",__FUNCTION__,__LINE__);
    return 0;
}
static int dmisc1_release(struct inode *inode, struct file *filp)
{
    printk("%s %d\n",__FUNCTION__,__LINE__);
    return 0;
}
static ssize_t dmisc1_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	printk("%s %d\n",__FUNCTION__,__LINE__);
    return 1;
}

extern struct ion_device *giondev;  //来自ion_dummy_driver.c
extern int getfdfrommsdrv1(int usrfd, int kerfd);

static ssize_t dmisc1_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	int ret = 0, ionfd = 0;
	ret = copy_from_user(&ionfd, buf, count);
	printk("%s %d, ionfd 0x%x\n",__FUNCTION__,__LINE__, ionfd);
#if 1
	/*---------------------------------------------
	* 打印user层下发的fd buf
	*/
	struct ion_client *client = ion_client_create(giondev, "dmisc1");
	if (IS_ERR(client)) {
		printk("%s %d, ion_client_create error\n",__FUNCTION__,__LINE__);
		return -1;
	}
	struct ion_handle *handle = ion_import_dma_buf_fd(client, ionfd);
	if (IS_ERR(handle)) {
		printk("%s %d, ion_import_dma_buf error\n",__FUNCTION__,__LINE__);
		return -1;
	}
	void *vaddr = ion_map_kernel(client, handle);
	if (IS_ERR(vaddr)) {
		printk("%s %d, ion_map_kernel error\n",__FUNCTION__,__LINE__);
		return -1;
	}
	printk("%s %d, *vaddr \033[;31m0x%x %d\033[0m\n",__FUNCTION__,__LINE__, (char *)vaddr, *((int *)vaddr));
	ion_unmap_kernel(client, handle);
	ion_client_destroy(client);
#endif
	/*---------------------------------------------
	* kernel申请ion buffer
	*/
	client = ion_client_create(giondev, "dmisc1-2");
	if (IS_ERR(client)) {
		printk("%s %d, ion_client_create error\n",__FUNCTION__,__LINE__);
		return -1;
	}
	struct ion_allocation_data allocdata;
	unsigned int plen = 0x3200000;
	allocdata.len = plen;
	allocdata.align = 0;
	allocdata.heap_id_mask = 1 << ION_HEAP_TYPE_SYSTEM; 
	allocdata.flags = 0;
	handle = ion_alloc(client, allocdata.len, allocdata.align, allocdata.heap_id_mask, allocdata.flags);
	if (IS_ERR(handle)) {
		printk("%s %d, ion_alloc error\n",__FUNCTION__,__LINE__);
	}
	int kerfd = ion_share_dma_buf_fd(client, handle);
	vaddr = ion_map_kernel(client, handle);
	if (IS_ERR(vaddr)) {
		printk("%s %d, ion_map_kernel error\n",__FUNCTION__,__LINE__);
		return -1;
	}
	printk("%s %d, [1]*vaddr 0x%x %d\n",__FUNCTION__,__LINE__, (char *)vaddr, *((int *)vaddr));
	*((int *)vaddr) = 33;
	printk("%s %d, [2]*vaddr \033[;33m0x%x %d\033[0m, kerfd \033[;33m0x%x\033[0m, ionfd \033[;32m0x%x\033[0m\n",
				__FUNCTION__,__LINE__, (char *)vaddr, *((int *)vaddr), kerfd, ionfd);
	ion_unmap_kernel(client, handle);
	ion_free(client, handle);
	ion_client_destroy(client);
	/*---------------------------------------------
	* 发送 user的ionfd、kernel的kerfd 到 dmisc2
	*/
	if (getfdfrommsdrv1(ionfd, kerfd))
		printk("%s %d, getfdfrommsdrv1 error\n",__FUNCTION__,__LINE__);
	//
    return 1;
}
static long dmisc1_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    printk("%s %d\n",__FUNCTION__,__LINE__);
    return 1;
}

static struct file_operations mdrv1_fops =
{
    .owner = THIS_MODULE,
    .open = dmisc1_open,
    .read = dmisc1_read,
    .write = dmisc1_write,
    .unlocked_ioctl = dmisc1_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = dmisc1_ioctl,
#endif
    .release = dmisc1_release
};

static struct miscdevice mdrv1_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "miscdev1",
	.fops = &mdrv1_fops,
};
static int __init mdrv1_init(void)
{
	printk("%s %s %d\n",__FILE__,__FUNCTION__,__LINE__);
	misc_register(&mdrv1_dev);
	return 0;
}
static void __exit mdrv1_exit(void)
{
	printk("%s %s %d\n",__FILE__,__FUNCTION__,__LINE__);
	misc_deregister(&mdrv1_dev);
}
module_init(mdrv1_init);
module_exit(mdrv1_exit);
MODULE_INFO(intree, "Y");
MODULE_LICENSE("GPL");