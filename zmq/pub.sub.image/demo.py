#!/usr/bin/env python3
#-*- coding: utf-8 -*-
from PIL import Image
import numpy as np
import struct, cv2

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

filename = "ship640x480.jpg"
filename2 = "img1920x1080.jpg"

img = cv2.imread(filename)
print(type(img), len(img), img.size, img.shape)
fm = "IIIIIII"
buffer = struct.pack(fm, 0, img.shape[1], img.shape[0], 8, 8, 0, 28)
print(type(buffer), len(buffer), buffer)
print(type(img.tobytes()), len(img.tobytes()), img.tobytes()[:10])

tw = buffer + img.tobytes()
print(type(tw), len(tw), tw[:40])

with open('ship640x480.bin', 'wb') as wfd:
	wfd.write(tw)