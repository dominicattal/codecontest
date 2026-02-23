#!/usr/bin/env python3

import sys
a = list(map(int,sys.stdin.readline().split()))
n = a[0]
a = a[1:]

# max[i] = smallest value in a[0]...a[i]:
max = [0 for i in range(n)]
max[0] = a[0]
for i in range(1,n):
  if a[i] > max[i-1]:
    max[i] = a[i]
  else:
    max[i] = max[i-1]

sol = []

minim = 1e10
for i in reversed(range(n)):
  # for each i, min = smallest value in a[i+1] ... (or infinity if i = n-1)
  if a[i] < minim:
    minim = a[i]
    if a[i] >= max[i]:
      sol.append(a[i])
sol.reverse()
lim =min(100,len(sol))
print(len(sol),' '.join(map(str,sol[:lim])))
