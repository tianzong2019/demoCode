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
	printf("SUB [%d] w.h.d.t %d %d %d %d, addr %p 0x%x %d\n", idx,
		data->w, data->h, data->d, data->t,
		data, data->offset, *(addr + idx%(pw*ph)));
}

int main(void)
{
    void *ctx,*sock;
    int ret = 0, i = 0;
    ctx = zmq_ctx_new();
    sock = zmq_socket(ctx,ZMQ_SUB);
    zmq_setsockopt(sock,ZMQ_SUBSCRIBE,"",0);
    ret = zmq_connect(sock,"tcp://127.0.0.1:5555");
    while(1)
    {
        struct frmsg *data = (struct frmsg *)malloc(psize);
        zmq_recv(sock, data, psize, 0);
        printfrmsg(data, i++);
        free(data);
    }
    zmq_close(sock);
    zmq_ctx_destroy(ctx);
    return 0;
}
