def matmul(m1, m2):
    MOD = 10**9+7
    res = [[0,0],[0,0]]
    res[0][0] = (m1[0][0] * m2[0][0] + m1[0][1] * m2[1][0]) % MOD
    res[0][1] = (m1[0][0] * m2[0][1] + m1[0][1] * m2[1][1]) % MOD
    res[1][0] = (m1[1][0] * m2[0][0] + m1[1][1] * m2[1][0]) % MOD
    res[1][1] = (m1[1][0] * m2[0][1] + m1[1][1] * m2[1][1]) % MOD
    return res

def fib_from_pow10(n):
    ans = [[1, 1], [1, 0]]
    for i in range(n):
        base = [[ans[0][0],ans[0][1]],[ans[1][0],ans[1][1]]]
        for _ in range(9):
            ans = matmul(ans, base)
    return ans[1][0]

print(fib_from_pow10(1000000))
