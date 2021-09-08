#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/version.h>

/*
echo -n "file dytimer.c  +p" > /sys/kernel/debug/dynamic_debug/control
echo -n "file dytimer.c  -p" > /sys/kernel/debug/dynamic_debug/control
echo -n 'module dytimer +p' > /sys/kernel/debug/dynamic_debug/control
echo -n 'module dytimer -p' > /sys/kernel/debug/dynamic_debug/control
echo -n 'func timerfunc +p' > /sys/kernel/debug/dynamic_debug/control
echo -n 'func timerfunc -p' > /sys/kernel/debug/dynamic_debug/control
echo -n 'func showtags +p' > /sys/kernel/debug/dynamic_debug/control
echo -n 'func showtags -p' > /sys/kernel/debug/dynamic_debug/control
echo -n 'format DTAG +p' > /sys/kernel/debug/dynamic_debug/control
echo -n 'format DTAG -p' > /sys/kernel/debug/dynamic_debug/control

cat /sys/kernel/debug/dynamic_debug/control |grep dytim

默认情况下，debugfs会被挂载在目录/sys/kernel/debug之下，如果没有自动挂载，可以用如下命令手动完成：
# mount -t debugfs none /sys/kernel/debug

rmmod dytimer

*/

static struct timer_list test_timer;

#define prdg(fmt, ...) \
        pr_debug("11DTAG" pr_fmt(fmt), ##__VA_ARGS__)
#define prdg2(fmt, ...) \
        pr_debug("22DTAG2" pr_fmt(fmt), ##__VA_ARGS__)

static struct timer_list test_timer;

static void showtags(void)
{
	prdg("showtags, 1111111111111111111111111111111111\n");
}

static void showtags22222(void)
{
	prdg2("showtags22222, 222222222222222222222222222222\n");
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
static void timerfunc(unsigned long data)
#else
static void timerfunc(struct timer_list  *timer)
#endif
{
	printk("22 again in timerfunc\n");
	pr_debug("prdebug test==========================================\n");
	mod_timer(&test_timer, jiffies + HZ);
	showtags();
	showtags22222();
}

static int timer_drv_init(void)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
	printk("%s %s %d %d\n",__FILE__,__FUNCTION__,__LINE__, LINUX_VERSION_CODE);
	init_timer(&test_timer);                     //初始化内核定时器
	test_timer.function = timerfunc;            //指定定时时间到后的回调函数
    test_timer.data = (unsigned long)"hello world";  //回调函数的参数
    test_timer.expires = jiffies + 1 * HZ;        //定时时间 1s 
#else
	test_timer.expires  = jiffies + 1 * HZ;
	timer_setup(&test_timer, timerfunc, 0);
#endif
	
	add_timer(&test_timer);
	
	printk(KERN_INFO "already init and add timer\n");
	
	return 0;
}

static void timer_drv_exit(void)
{
	printk(KERN_INFO "exit timer drv\n");
	del_timer(&test_timer);
}

module_init(timer_drv_init);
module_exit(timer_drv_exit);
MODULE_LICENSE("GPL");