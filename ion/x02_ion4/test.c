#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>

# if __WORDSIZE == 64
typedef long int              int64_t;
# else
__extension__
typedef long long int      int64_t;
#endif

#include "/home/albter/codespace/qemu.arm/linux-5.10/drivers/staging/android/uapi/ion.h"

#if 0
struct ion_allocation_data {
    __u64 len;
    __u32 heap_id_mask;
    __u32 flags;
    __u32 fd;
    __u32 unused;
};
struct ion_heap_query {
    __u32 cnt; /* Total number of heaps to be copied */
    __u32 reserved0; /* align to 64bits */
    __u64 heaps; /* buffer to be populated */
    __u32 reserved1;
    __u32 reserved2;
};
struct ion_heap_data {
    char name[MAX_HEAP_NAME];
    __u32 type;
    __u32 heap_id;
    __u32 reserved0;
    __u32 reserved1;
    __u32 reserved2;
};
#endif

void queryion(int fd)
{
	int cnt = 0, i = 0;
	struct ion_heap_query query;
	//
	memset(&query, 0, sizeof(query));
    if (ioctl(fd, ION_IOC_HEAP_QUERY, &query)) {
		printf("%s %d, ION_IOC_HEAP_QUERY cnt error\n", __FUNCTION__, __LINE__);
		return;
	}
    cnt = query.cnt;
	printf("%s %d, heap cnt %d\n", __FUNCTION__, __LINE__, cnt);
	//
	struct ion_heap_data *buffers = (struct ion_heap_data *)malloc(cnt * sizeof(struct ion_heap_data));
	query.heaps = (__u64)buffers;
	if (ioctl(fd, ION_IOC_HEAP_QUERY, &query)) {
		printf("%s %d, ION_IOC_HEAP_QUERY cnt error\n", __FUNCTION__, __LINE__);
		return;
	}
	for (i = 0; i < cnt; i++) {
		struct ion_heap_data *dat = (struct ion_heap_data *)buffers;
		printf("%s %d, heap.name.type.id %s %d %d\n", __FUNCTION__, __LINE__, dat[i].name, dat[i].type, dat[i].heap_id);
	}
/* 
queryion 54, heap cnt 3
queryion 64, heap.name.type.id reserved 4 2
queryion 64, heap.name.type.id ion_system_heap 0 1         arm32 最大882.1875M
queryion 64, heap.name.type.id ion_system_contig_heap 1 0  arm32 KMALLOC_MAX_SIZE是4M
*/
}

int ionbuffer(void)
{
	int cnt = 3;
	/*
	* 32M 0x2000000, 2048M 0x80000000， 882M 0x37200000, 128M 0x8000000
	* 8M 0x800000, 4M 0x400000, 862M 0x35E00000, 800M 0x32000000, 400M 0x19000000
	*/
	unsigned int plen = 0x400000; 
	struct ion_allocation_data adata;
	//
	int fd = open("/dev/ion", O_RDWR);
	if (fd == -1) {
		printf("%s %d, open /dev/new-miscdev error\n", __FUNCTION__, __LINE__);
		return -1;
	}
	//
	queryion(fd);
	//
	adata.len=plen;
	adata.heap_id_mask = 1 << 1;//ION_HEAP_TYPE_SYSTEM;
	//adata.flags = ION_HEAP_TYPE_SYSTEM;
	if (ioctl(fd, ION_IOC_ALLOC, &adata)) {
		printf("%s %d, ION_IOC_ALLOC error\n", __FUNCTION__, __LINE__);
		return -1;
	}
	//
	int *p = mmap(0, plen, PROT_READ|PROT_WRITE, MAP_SHARED, adata.fd, 0);
	p[0]=99;
	perror("test");
	printf("hello all %d\n",p[0]);
	munmap(p, plen);
	//
	close(fd);
	//
	p = mmap(0, plen, PROT_READ|PROT_WRITE, MAP_SHARED, adata.fd, 0);
	printf("hello all %d\n",p[0]);
	munmap(p, plen);
	//
	return adata.fd;
}

int main(int argc, char const *argv[])
{
	int fd = -1;

	printf("%s %d\n", __FUNCTION__, __LINE__);
	//读写ion buffer
	int ionfd = ionbuffer();
	printf("%s %d, ionfd 0x%x\n", __FUNCTION__, __LINE__, ionfd);
	//
	fd = open("/dev/miscdev", O_RDWR);
	if (fd == -1) {
		printf("%s %d, open /dev/miscdev error\n", __FUNCTION__, __LINE__);
		return -1;
	}
	write(fd, &ionfd, sizeof(ionfd));
	close(fd);
	//
    return 0;
}