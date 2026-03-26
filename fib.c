#include <stdio.h>

#define MOD 1000000007

typedef long long ll;
typedef struct {
    ll v00,v01,v10,v11;
} mat;

mat matmul(mat m1, mat m2)
{
    mat res;
    res.v00 = (m1.v00 * m2.v00 + m1.v01 * m2.v10) % MOD;
    res.v01 = (m1.v00 * m2.v01 + m1.v01 * m2.v11) % MOD;
    res.v10 = (m1.v10 * m2.v00 + m1.v11 * m2.v10) % MOD;
    res.v11 = (m1.v10 * m2.v01 + m1.v11 * m2.v11) % MOD;
    return res;
}

ll fib(ll n)
{
    mat ans = {1, 1, 1, 0};
    mat cur = {1, 1, 1, 0};
    while (n > 0) {
        if (n & 1)
            ans = matmul(ans, cur);
        cur = matmul(cur, cur);
        n >>= 1;
    }
    return ans.v11;
}

ll fib_from_pow2(ll n)
{
    mat ans = {1, 1, 1, 0};
    for (ll i = 0; i < n; i++)
        ans = matmul(ans, ans);
    return ans.v10;
}

ll fib10(ll n)
{
    mat ans = {1, 1, 1, 0};
    mat cur = {1, 1, 1, 0};
    while (n > 0) {
        for (int i = 0; i < n%10; i++)
            ans = matmul(ans, cur);
        mat base = cur;
        for (int i = 0; i < 9; i++)
            cur = matmul(cur, base);
        n /= 10;
    }
    return ans.v11;
}

ll fib_from_pow10(ll n)
{
    mat ans = {1, 1, 1, 0};
    for (int i = 0; i < n; i++) {
        mat base = ans;
        for (int j = 0; j < 9; j++)
            ans = matmul(ans, base);
    }
    return ans.v10;
}

void main()
{
    printf("%llu\n", fib_from_pow10(1000000));
}
