#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <zmq.h>
#include <unistd.h>
#include <string.h>
#include "comm.h"

void printfrmsg(struct frmsg *data, int idx)
{
	unsigned short int *addr = (unsigned short int *)((void *)data + data->offset);
	printf("[%d] w.h.d.t %d %d %d %d, addr %p 0x%x %d\n", idx,
		data->w, data->h, data->d, data->t,
		data, data->offset, *(addr + idx%(pw*ph)));
}

struct frmsg *getdata(void)
{
	int i = 0;
	unsigned short int *addr = NULL;
    struct frmsg *data = (struct frmsg *)malloc(psize);
    data->w = pw;
	data->h = ph;
	data->d = pd;
	data->t = pt;
	data->size = pw*ph*pt/8;
	data->offset = sizeof(struct frmsg);
	addr = (unsigned short int *)((void *)data + data->offset);
    for (i=0; i < pw*ph; i++)
        *(addr + i) = i*4;
    return data;
}

int main(void)
{
    void *ctx,*sock;
    int ret = 0, i=0;
    ctx = zmq_ctx_new();
    sock = zmq_socket(ctx,ZMQ_PUB);
    ret = zmq_bind(sock,"tcp://127.0.0.1:5555");
    while(1)
    {
        struct frmsg *data = getdata();
		printfrmsg(data, i++);
        ret = zmq_send(sock, data, psize,0);
        sleep(1);
        free(data);
    }
    zmq_close(sock);
    zmq_ctx_destroy(ctx);
    return 0;
}
