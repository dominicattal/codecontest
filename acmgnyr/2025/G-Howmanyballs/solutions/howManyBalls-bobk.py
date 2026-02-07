#!/usr/bin/python3
import math

(p,q) = map(int, input().split(' '))

r = 0
g = 0
for i in range(1,1000000):
    a = p
    b = 2 * i * (p - q) - p
    c = p * i * (i - 1)
    d = b * b - 4 * a * c
    if d < 0:
        continue
    x1 = int((-b + math.sqrt(d)) / (2 * a) + 0.5)
    x2 = int((-b - math.sqrt(d)) / (2 * a) + 0.5)
    if x1 > i and 2 * i * x1 * q == (i + x1) * (i + x1 - 1) * p:
        r = i
        g = x1
        break
    if x2 > i and 2 * i * x2 * q == (i + x2) * (i + x2 - 1) * p:
        r = i
        g = x2
        break

if r == 0:
    print('impossible')
else:
    print(r, g)
