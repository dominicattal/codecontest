def gcd(a,b):
    if b == 0:
        return a
    else:
        return gcd(b,a % b)
    
import math
n = int(input())
pre = int((1+math.sqrt(8*n+1))/2)
exact = int(pre*(pre-1)/2)
# would be easier if we started numbering at 0 :)
if n == exact+1:
    print(pre)
elif n > exact:
    numer = n-(exact+1)
    d = gcd(pre,numer)
    print(str(pre)+" "+str(numer//d) + "/" +str(pre//d))
elif n == 1:
    print(1)
else:
    print(str(pre-1)+" "+str(pre-2) + "/" +str(pre-1))

