#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>


#include "/home/albter/codespace/qemu.arm/linux-4.11/drivers/staging/android/uapi/ion.h"


/*=========================================================
* 1. 申请ion buf
* 2. 发送ion fd到fromkern2
* 3. 下发ion fd到kernel
*/
void *tokern1(void *arg)
{
	int i = 0;
	int *wpfd = (int *)arg;
	printf("%s %d, begin...\n", __FUNCTION__, __LINE__);
	//
	int fd = open("/dev/ion",O_RDWR);
	if (fd == -1) {
		printf("%s %d, open ion\n", __FUNCTION__, __LINE__);
        return NULL;
	}
	/*---------------------------------------------
	* query, struct ion_heap_query query;
	*/
	struct ion_heap_query query;
	if (ioctl(fd, ION_IOC_HEAP_QUERY, &query)) {
		printf("%s %d, QUERY error\n", __FUNCTION__, __LINE__);
        return NULL;
	}
	printf("%s %d, cnt %d\n", __FUNCTION__, __LINE__, query.cnt);
	struct ion_heap_data *hdata = (struct ion_heap_data *)malloc(query.cnt * sizeof(struct ion_heap_data));
	query.heaps = (__u64)hdata;
	if (ioctl(fd, ION_IOC_HEAP_QUERY, &query)) {
		printf("%s %d, QUERY error\n", __FUNCTION__, __LINE__);
        return NULL;
	}
	for (i = 0; i < query.cnt; i++)
		printf("%s %d, hdata.name, %s, %d, %d\n", __FUNCTION__, __LINE__, (hdata + i)->name, (hdata + i)->type, (hdata + i)->heap_id);
	/*---------------------------------------------
	* alloc, struct ion_allocation_data allocation;
	* max 883.9375M 0x373F0000
	* 900M 0x38400000; 880M 0x37000000; 50M 0x3200000
	*/
	unsigned int plen = 0x3200000;
	struct ion_allocation_data allocdata;
	
	allocdata.len = plen;
	allocdata.align = 0;
	allocdata.heap_id_mask = 1 << ION_HEAP_TYPE_SYSTEM; 
	allocdata.flags = 0;
	if (ioctl(fd, ION_IOC_ALLOC, &allocdata)) {
		printf("%s %d, ALLOC error\n", __FUNCTION__, __LINE__);
        return NULL;
	}

	struct ion_fd_data ionfd;
	ionfd.handle = allocdata.handle;
	if (ioctl(fd, ION_IOC_SHARE, &ionfd)) {
		printf("%s %d, SHARE error\n", __FUNCTION__, __LINE__);
        return NULL;
	}

	int *p = mmap(0, plen, PROT_READ|PROT_WRITE, MAP_SHARED, ionfd.fd, 0);
	p[0] = 95;
	printf("%s %d, [1]*p \033[;31m0x%x %d\033[0m\n", __FUNCTION__, __LINE__, p, *p);
	munmap(p, plen);
	close(fd);
	/*---------------------------------------------
	* 把ionfd.fd发到fromkern2
	*/
	printf("%s %d, 发送ionfd.fd \033[;32m0x%x\033[0m 到fromkern2\n", __FUNCTION__, __LINE__, ionfd.fd);
	write(*wpfd, &ionfd.fd, sizeof(ionfd.fd));
	close(*wpfd);
	/*---------------------------------------------
	* 下发ion fd到kernel
	*/
	printf("%s %d, 发送ionfd.fd \033[;32m0x%x\033[0m 到kernel\n", __FUNCTION__, __LINE__, ionfd.fd);
	fd = open("/dev/miscdev1", O_RDWR);
	if (fd == -1) {
		printf("%s %d, open miscdev1 error\n", __FUNCTION__, __LINE__);
        return NULL;
	}
	write(fd, &ionfd.fd, sizeof(ionfd.fd));
	close(fd);
	//
    return NULL;
}

/*=========================================================
* 1.接收tokern1发来的ion fd
* 2.接收mdrv2发来的ion fd，并比对1的接收
* 3.compare ok，打印mdrv2发来的内容
*/
void *fromkern2(void *arg)
{
	int ionfd, ionfd2;
	unsigned int plen = 0x3200000;
	int *rpfd = (int *)arg;
	printf("%s %d, begin...\n", __FUNCTION__, __LINE__);
	read(*rpfd, &ionfd, sizeof(ionfd));
	close(*rpfd);
	printf("%s %d, ionfd 0x%x\n", __FUNCTION__, __LINE__, ionfd);
	/*---------------------------------------------
	* mmap ionfd
	*/
	int *p = mmap(0, plen, PROT_READ|PROT_WRITE, MAP_SHARED, ionfd, 0);
	printf("%s %d, *p \033[;31m0x%x %d\033[0m\n", __FUNCTION__, __LINE__, p, *p);
	munmap(p, plen);
	/*---------------------------------------------
	* 读取kern数据
	*/
	int fd = open("/dev/miscdev2", O_RDWR);
	if (fd == -1) {
		printf("%s %d, open miscdev1 error\n", __FUNCTION__, __LINE__);
        return NULL;
	}
	read(fd, &ionfd2, sizeof(ionfd2));
	printf("%s %d, ionfd2 \033[;32m0x%x\033[0m\n", __FUNCTION__, __LINE__, ionfd2);
	close(fd);
	//
	if (ionfd2 == ionfd) {
		p = mmap(0, plen, PROT_READ|PROT_WRITE, MAP_SHARED, ionfd2, 0);
		printf("%s %d, read from kernel *p \033[;33m0x%x %d\033[0m\n", __FUNCTION__, __LINE__, p, *p);
		munmap(p, plen);
	}
	close(ionfd2);
	//
    return NULL;
}
/*=========================================================
* 1. 创建pipe用于tokern1 和 fromkern2间 ion fd传输
* 2. 创建线程tokern1，与mdrv1通信，下发ion fd
* 3. 创建线程fromkern2，与mdrv2通信，接收ion fd
* 4. join线程tokern1、fromkern2
*/
int main(int argc, char *argv[])
{
	int ret;
	int pipefd[2]; //0 read; 1 write
	pthread_t myThread1,myThread2;
	//
	if (pipe(pipefd) == -1) {
        printf("%s %d, pipe fail\n", __FUNCTION__, __LINE__);
        return -1;
    }
	//
    ret = pthread_create(&myThread1, NULL, tokern1, (void *)(pipefd + 1));
    if (ret != 0) {
		printf("%s %d, 线程创建失败\n", __FUNCTION__, __LINE__);
        return ret;
    }
	ret = pthread_create(&myThread2, NULL, fromkern2, (void *)(pipefd));
    if (ret != 0) {
		printf("%s %d, 线程创建失败\n", __FUNCTION__, __LINE__);
        return ret;
    }
    pthread_join(myThread1, NULL);
	pthread_join(myThread2, NULL);
    return 0;
}
