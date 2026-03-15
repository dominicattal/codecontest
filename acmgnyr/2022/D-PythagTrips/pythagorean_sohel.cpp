/**
 * Problem Name: Counting Pythagorean Triples
 * Author: Sohel Hafiz
 * Date: 2/20/2023 
 * Contest: GNY Regional, 2022
*/

#include<bits/stdc++.h>
 
using namespace std;
 
int gcd(long long a, long long b) {
    if (b == 0) return a;
    return gcd(b, a % b);
}

int main() {
    int n;
    cin >> n;
    assert (n >= 3 && n <= 2500);

    int r1, r2;
    r1 = r2 = 0;
    for (int a = 1; a <= n; a++)
        for (int b = a; b <= n; b++) {
            if (a*a + b*b == n*n) {
               if (gcd( gcd(a, b), n) == 1) {
                    r1++;
               } else {
                    r2++;
               }
           }

        }

    int r3, r4;
    r3 = r4 = 0;
    for (long long a = 1; a <= 10000000; a++) {
        long long s = a*a + n*n;
        long long root = sqrt(s);
        bool yes = false;
        long long jj = -1;
        for (long long int j = root - 1; j <= root + 1; j++) {
            if (j * j == s) yes = true, jj = j;
        }
        if (yes) {
            if (gcd(gcd(a, n), jj) == 1) {
                r3++;
            } else {
                r4++;
            }
        }
    }
    cout << r1 << " " << r2 << " " << r3 << " " << r4 << endl;
	return 0;
}
