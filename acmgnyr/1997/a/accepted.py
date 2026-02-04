import sys
t = int(input())
for _ in range(t):
    n = int(input())
    b1 = n&0xFF
    b2 = (n>>8)&0xFF
    b3 = (n>>16)&0xFF
    b4 = (n>>24)&0xFF
    r = (b1<<24)+(b2<<16)+(b3<<8)+b4
    if r > (1<<31):
        r = -2*(1<<31) + r
    print(f"{n} converts to {r}")
