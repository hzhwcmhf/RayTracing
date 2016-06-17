import numpy as np
import Image
import ImageFilter 

path = 'data/finalResultWithoutBrightness%d.txt'
width = 600
height = 600

def readData(id):
	f = open(path % id)
	w, h = map(int, f.readline().strip().split())
	ans = map(lambda x: map(lambda y:map(lambda z: float(z),f.readline().strip().split()), range(h)), range(w))
	f.close()
	print 'read ok:', id
	return ans

def calAverage(start, end):
	d = None
	for i in range(start, end):
		a = np.array(readData(i))
		if i == start:
			d = a
		else:
			d += a
	return d / (end - start)

def calMSE(start , end, aver):
	p = []
	for i in range(start, end):
		a = np.array(readData(i))
		p.append((i, np.sum((a - aver) ** 2)))
	return p

def sortMSE(start , end):
	aver = calAverage(start, end)
	p = calMSE(start, end, aver)
	p.sort(key = lambda x:x[1])
	return p
	
def makeImage(p):
	d = None
	for i in p:
		a = np.array(readData(i))
		if i == p[0]:
			d = a
		else:
			d += a
	mid = np.median(d)
	
	d = d / mid * 255 * 0.6
	#print d[200][200]
	
	im  = Image.new('RGB', (width, height))
	pic = im.load()
	
	
	for j in range(width):
		for k in range(height):
			#print d[j][k] / mid * 255 * 0.6
			pic[j,height -1 - k] = tuple(map(lambda x: [255,int(x)][x < 255],reversed(d[j][k].tolist())))
	print pic[200,200]
	return im

def work1():
	p = sortMSE(12670,12690)
	print p
	p = map(lambda x:x[0], p)
	im = makeImage(p[0:15])
	im.save('makephoto1.bmp')
	im = makeImage(p[0:18])
	im.save('makephoto2.bmp')
	im = makeImage(p[0:20])
	im.save('makephoto3.bmp')

def work2():
	im = Image.open('makephoto.bmp')
	#im2 = im.filter(ImageFilter.MedianFilter())
	im2 = im.filter(ImageFilter.MedianFilter())
	#im2 = im2.filter(ImageFilter.SMOOTH())
	#im2 = im2.filter(ImageFilter.SMOOTH_MORE())
	#im2 = im2.filter(ImageFilter.SHARPEN())
	#im2 = im2.filter(ImageFilter.EDGE_ENHANCE())
	
	im2.save('makephoto1.bmp')

#work1()
work2()