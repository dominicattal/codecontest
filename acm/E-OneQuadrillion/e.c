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

int main()
{
    printf("%lld\n", fib(1000000000000000LL));
}
