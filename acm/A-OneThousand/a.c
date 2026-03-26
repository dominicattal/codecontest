#include <stdio.h>

int main()
{
    int n = 1000;
    int F[1001] = {0};
    F[1] = 1;
    for (int i = 2; i <= n; i++)
        F[i] = (F[i-1] + F[i-2]) % 1000000007;
    printf("%d\n", F[n]);
}
