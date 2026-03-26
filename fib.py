def matmul(m1, m2):
    MOD = 10**9+7
    res = [[0,0],[0,0]]
    res[0][0] = (m1[0][0] * m2[0][0] + m1[0][1] * m2[1][0]) % MOD
    res[0][1] = (m1[0][0] * m2[0][1] + m1[0][1] * m2[1][1]) % MOD
    res[1][0] = (m1[1][0] * m2[0][0] + m1[1][1] * m2[1][0]) % MOD
    res[1][1] = (m1[1][0] * m2[0][1] + m1[1][1] * m2[1][1]) % MOD
    return res

def fib(n):
    ans = [[1, 1], [1, 0]]
    cur = [[1, 1], [1, 0]]
    while n > 0:
        if n & 1:
            ans = matmul(ans, cur)
        cur = matmul(cur, cur)
        n >>= 1
    return ans[1][1]

def fib_from_pow2(n):
    ans = [[1, 1], [1, 0]]
    for i in range(n):
        ans = matmul(ans, ans)
    return ans[1][0]

def fib10(n):
    ans = [[1, 1], [1, 0]]
    cur = [[1, 1], [1, 0]]
    while n > 0:
        for _ in range(n%10):
            ans = matmul(ans, cur)
        base = [[cur[0][0],cur[0][1]],[cur[1][0],cur[1][1]]]
        for _ in range(9):
            cur = matmul(cur, base)
        n //= 10
    return ans[1][1]

def fib_from_pow10(n):
    ans = [[1, 1], [1, 0]]
    for i in range(n):
        base = [[ans[0][0],ans[0][1]],[ans[1][0],ans[1][1]]]
        for _ in range(9):
            ans = matmul(ans, base)
    return ans[1][0]

#print(fib(10**1000000)) 305562778
#print(fib(2**20))
#print(fib_from_pow2(20))
#print(fib(123456789))
#print(fib10(123456789))
#print(fib10(10**10))
print(fib_from_pow10(1000000))

