import struct
import math
import os
import sys
from PIL import Image

srcfn=sys.argv[1]
dstname=sys.argv[2]

im = Image.open(srcfn)
imout = Image.new("RGBA", (8,1024))

w, h = im.size

for k in range(96):
	mx=7
	my=0
	for j in range(16):
		for i in range(16):
			if im.getpixel((k*16+i,j))[3]!=0:
				mx=min(mx,i)
				my=min(my,j)
	for j in range(8):
		for i in range(8):
			# pixel = im.getpixel((k*16+mx+i,j+my))
			pixel = im.getpixel((k*16+mx+i,j))
			imout.putpixel((i,k*8+j), pixel)

imout.save(dstname)
