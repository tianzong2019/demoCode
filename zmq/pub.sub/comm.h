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

#define pw 6
#define ph 4
#define pd 12
#define pt 16

#define psize (sizeof(struct frmsg) + pw*ph*pt/8)

#endif
