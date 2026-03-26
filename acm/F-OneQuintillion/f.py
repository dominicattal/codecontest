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

print(fib(10**18))
