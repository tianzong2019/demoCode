#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/dma-buf.h>

#include "/home/albter/codespace/qemu.arm/linux-4.11/drivers/staging/android/ion/ion.h"

int gusrfd = 0, gkerfd = 0;
DECLARE_COMPLETION(comp);

int getfdfrommsdrv1(int usrfd, int kerfd)
{
    int ret = 0;
	printk("%s %d, usrfd \033[;32m0x%x\033[0m, kerfd \033[;33m0x%x\033[0m\n",__FUNCTION__,__LINE__, usrfd, kerfd);
	gusrfd = usrfd;
	gkerfd = kerfd;
	complete(&comp);
	//
    return ret;
}                     
EXPORT_SYMBOL(getfdfrommsdrv1);

static int dmisc2_open(struct inode *inode, struct file *filp)
{
    printk("%s %d\n",__FUNCTION__,__LINE__);
    return 0;
}
static int dmisc2_release(struct inode *inode, struct file *filp)
{
    printk("%s %d\n",__FUNCTION__,__LINE__);
    return 0;
}

extern struct ion_device *giondev;  //来自ion_dummy_driver.c
static ssize_t dmisc2_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	int ret = 0;
	int buffer[10];
	//
	printk("%s %d\n",__FUNCTION__,__LINE__);
	wait_for_completion(&comp);
	printk("%s %d, usrfd \033[;32m0x%x\033[0m, kerfd \033[;33m0x%x\033[0m\n",__FUNCTION__,__LINE__, gusrfd, gkerfd);
	/*---------------------------------------------
	* 读取kern ion data写入user ion buffer
	*/
	struct ion_client *client = ion_client_create(giondev, "dmisc2");
	if (IS_ERR(client)) {
		printk("%s %d, ion_client_create error\n",__FUNCTION__,__LINE__);
		return -1;
	}
	struct ion_handle *handle = ion_import_dma_buf_fd(client, gkerfd);
	if (IS_ERR(handle)) {
		printk("%s %d, ion_import_dma_buf error\n",__FUNCTION__,__LINE__);
		return -1;
	}
	void *vaddr = ion_map_kernel(client, handle);
	if (IS_ERR(vaddr)) {
		printk("%s %d, ion_map_kernel error\n",__FUNCTION__,__LINE__);
		return -1;
	}
	printk("%s %d, *vaddr \033[;33m0x%x %d\033[0m\n",__FUNCTION__,__LINE__, (char *)vaddr, *((int *)vaddr));
	buffer[0] = *((int *)vaddr);
	ion_unmap_kernel(client, handle);
	ion_free(client, handle);
	ion_client_destroy(client);
	printk("%s %d, buffer[0] \033[;33m0x%x %d\033[0m\n",__FUNCTION__,__LINE__, *((int *)buffer));
	//
	//写入user ion buffer
	client = ion_client_create(giondev, "dmisc2-2");
	if (IS_ERR(client)) {
		printk("%s %d, ion_client_create error\n",__FUNCTION__,__LINE__);
		return -1;
	}
	handle = ion_import_dma_buf_fd(client, gusrfd);
	if (IS_ERR(handle)) {
		printk("%s %d, ion_import_dma_buf error\n",__FUNCTION__,__LINE__);
		return -1;
	}
	vaddr = ion_map_kernel(client, handle);
	if (IS_ERR(vaddr)) {
		printk("%s %d, ion_map_kernel error\n",__FUNCTION__,__LINE__);
		return -1;
	}
	printk("%s %d, *vaddr \033[;31m0x%x %d\033[0m\n",__FUNCTION__,__LINE__, (char *)vaddr, *((int *)vaddr));
	*((int *)vaddr) = *buffer;
	printk("%s %d, *vaddr \033[;33m0x%x %d\033[0m\n",__FUNCTION__,__LINE__, (char *)vaddr, *((int *)vaddr));
	ion_unmap_kernel(client, handle);
	ion_client_destroy(client);
	//
	ret = copy_to_user(buf, &gusrfd, sizeof(gusrfd));
    return 1;
}

static ssize_t dmisc2_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	printk("%s %d\n",__FUNCTION__,__LINE__);
    return 1;
}
static long dmisc2_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    printk("%s %d\n",__FUNCTION__,__LINE__);
    return 1;
}

static struct file_operations mdrv2_fops =
{
    .owner = THIS_MODULE,
    .open = dmisc2_open,
    .read = dmisc2_read,
    .write = dmisc2_write,
    .unlocked_ioctl = dmisc2_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = dmisc2_ioctl,
#endif
    .release = dmisc2_release
};

static struct miscdevice mdrv2_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "miscdev2",
	.fops = &mdrv2_fops,
};
static int __init mdrv2_init(void)
{
	printk("%s %s %d\n",__FILE__,__FUNCTION__,__LINE__);
	misc_register(&mdrv2_dev);
	return 0;
}
static void __exit mdrv2_exit(void)
{
	printk("%s %s %d\n",__FILE__,__FUNCTION__,__LINE__);
	misc_deregister(&mdrv2_dev);
}
module_init(mdrv2_init);
module_exit(mdrv2_exit);
MODULE_INFO(intree, "Y");
MODULE_LICENSE("GPL");