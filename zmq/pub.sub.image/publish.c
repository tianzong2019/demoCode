#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <zmq.h>
#include <unistd.h>
#include <string.h>
#include "comm.h"

void printfrmsg(struct frmsg *data, int idx)
{
	unsigned char *addr = (unsigned char *)((void *)data + data->offset);
	printf("[%d] w.h.d.t %d %d %d %d, addr %p 0x%x 0x%x\n", idx,
		data->w, data->h, data->d, data->t,
		data, data->offset, *(addr + idx%(pw*ph)));
}

struct frmsg *getdata(void)
{
	struct frmsg *data = (struct frmsg *)malloc(psize);
	data->offset = sizeof(struct frmsg);
	FILE * input = fopen("ship640x480.bin","rb");
	fread(data, 1, psize, input);
	fclose(input);
	return data;
}

int main(void)
{
    void *ctx,*sock;
    int ret = 0, i=0;
    ctx = zmq_ctx_new();
    sock = zmq_socket(ctx,ZMQ_PUB);
    ret = zmq_bind(sock,"tcp://127.0.0.1:5555");
	struct frmsg *data = getdata();
    while(1)
    {
		//struct frmsg *data = getdata();
		//printfrmsg(data, i++);
        ret = zmq_send(sock, data, psize,0);
        //sleep(1);
		//free(data);
    }
	free(data);
	
    zmq_close(sock);
    zmq_ctx_destroy(ctx);
    return 0;
}
