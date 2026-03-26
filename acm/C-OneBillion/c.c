#include <stdio.h>

#define MOD 1000000007
#define N 1000000000

int main()
{
    int f1, f2;
    f1 = 0;
    f2 = 1;
    for (int i = 2; i <= N; i++) {
        int tmp = f2;
        f2 = (f1 + f2) % MOD;
        f1 = tmp;
    }
    printf("%d\n", f2);
}
