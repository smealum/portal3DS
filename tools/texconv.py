import struct
import math
import os
import sys
from PIL import Image

tileOrder=[0,1,8,9,2,3,10,11,16,17,24,25,18,19,26,27,4,5,12,13,6,7,14,15,20,21,28,29,22,23,30,31,32,33,40,41,34,35,42,43,48,49,56,57,50,51,58,59,36,37,44,45,38,39,46,47,52,53,60,61,54,55,62,63]
def parseTile(im, l, x, y):
	global tileOrder
	for k in range(8*8):
		i=tileOrder[k]%8
		j=int((tileOrder[k]-i)/8)
		pixel=im.getpixel((x+i,y+j))
		l.append(((pixel[0]&0xFF)<<24)|((pixel[1]&0xFF)<<16)|((pixel[2]&0xFF)<<8)|(pixel[3]&0xFF))

srcfn=sys.argv[1]
dstname=sys.argv[2]

im = Image.open(srcfn)

w, h = im.size

# dstname=srcfn+".bin"

l=[]
for j in range(0,h,8):
	for i in range(0,w,8):
		parseTile(im, l, i, j)

dst=open(dstname,"wb")
for k in l:
	dst.write(struct.pack("I", k))
dst.close()
