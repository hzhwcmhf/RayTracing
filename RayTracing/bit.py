a = [1 for i in range(4*4)]

a[0*4+2] = 0
a[2*4+0] = 0

print a
print "%x" % int("".join(map(str, reversed(a))),2)

b = [1 for i in range(4*4*4)]

for i in range(3):
	for j in range(3):
		for k in range(3):
			if (i == 2 and j == 0) or (i == 0 and j == 2) or (i == 2 and k == 0) or (i == 0 and k == 2)	or (j == 2 and k == 0) or (j == 0 and k == 2):
				b[i*16+j*4+k] = 0
	
print "%x" % int("".join(map(str, reversed(b))),2)
