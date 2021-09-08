#ifndef _COMM_H
#define _COMM_H

struct frmsg {
    unsigned int fid;
    unsigned int w;
    unsigned int h;
    unsigned int d;
    unsigned int t;
    unsigned int size;
    unsigned int offset;
};

#define pw 640
#define ph 480
#define pd 8
#define pt 8*3

#define psize (sizeof(struct frmsg) + pw*ph*pt/8)

#endif
