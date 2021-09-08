#!/usr/bin/env python3
#-*-  coding:utf-8 -*-
import zmq, struct, cv2, time
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

time1 = time.time()
ct1 = ct2 = 0
cnt = 1
cv2.namedWindow("image",cv2.WINDOW_NORMAL)
while cnt:
	data = socket.recv()
	time2 = time.time()
	ct2 += 1
	print(ct2, (ct2-ct1)/(time2-time1))
	fm = "IIIIIII"
	response = struct.unpack_from(fm, data, 0)
	#print(type(data), len(data), response, response[-1])
	newdata = data[response[-1]:]
	#print("newdata", type(newdata), len(newdata))
	res = np.frombuffer(newdata, dtype='uint'+str(response[-3]), offset=0)
	#print(type(res), len(res), res.dtype, "0x%x 0x%x 0x%x 0x%x" %(res[0], res[1], res[2], res[3]))
	imgraw = res.reshape(480, 640, 3)
	cv2.imshow('image', imgraw)
	if cv2.waitKey(1) == 27:  # 特定的100ms
		cnt = 0
		cv2.destroyAllWindows()
		break