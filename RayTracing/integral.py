import random
RealWidth = 4
RealHeight = 3
ans = 0


x = random.uniform(-RealWidth/2, RealWidth/2)
y = random.uniform(-RealHeight/2, RealHeight/2)
z = 1
p = (x*x+y*y+z*z) ** 0.5
x /= p
y /= p
z /= p
ans += 1. / (z **3)