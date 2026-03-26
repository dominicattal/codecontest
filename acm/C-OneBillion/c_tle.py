n = 10**9
m = 10**9 + 7
f1, f2 = 0, 1
for i in range(2, n+1):
    f1, f2 = f2, (f1 + f2) % m
print(f2)
