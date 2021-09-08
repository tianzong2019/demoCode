#!/usr/bin/env python3
#-*-  coding:utf-8 -*-
import zmq, struct
import ctypes
import numpy as np

"""
struct frmsg {
    unsigned int fid;
    unsigned int w;
    unsigned int h;
    unsigned int d;
    unsigned int t;
    unsigned int size;
    unsigned int offset;
};
"""

context = zmq.Context()
socket = context.socket(zmq.SUB)
socket.connect("tcp://localhost:5555")
socket.setsockopt(zmq.SUBSCRIBE,''.encode('utf-8'))  # 接收所有消息
while True:
	data = socket.recv()
	fm = "IIIIIII"
	response = struct.unpack_from(fm, data, 0)
	print(type(data), len(data), response, response[-1])
	newdata = data[response[-1]:]
	print("newdata", type(newdata), len(newdata))
	res = np.frombuffer(newdata, dtype='uint16', offset=0)
	print(type(res), len(res), res.dtype, res)