#!/usr/bin/env python3

import sys
import math # only needed to get log_2 of input values

n = int(sys.stdin.readline())
a = list(map(int,sys.stdin.readline().split()))
# convert powers of 2 to exponents:
a = list(map(int,map(math.log2,a)))

t = [[0 for j in range(n)] for i in range(n)]
for i in range(n):
  t[i][i] = a[i]

for d in range(1,n): # diagonal
  for i in range(n-d):
    # we are filling in row i, column i+d
    j = i+d
    # Need to find max of i pairs of entries:
    m = 0
    for k in range(d):
      if t[i][i+k] == t[i+k+1][j]:
        m = max(m,1+t[i][i+k])
      else:
        m = max(m,t[i][i+k],t[i+k+1][j])
    t[i][j] = m

print(2**t[0][n-1])
