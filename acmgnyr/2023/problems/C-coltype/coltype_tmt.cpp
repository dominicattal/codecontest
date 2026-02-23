#include <bits/stdc++.h>
using namespace std;

int main() {
  string S;
  cin >> S;
  int m = S.size();
  // check first.
  if (S.back() != 'O') {
    cout << "INVALID" << endl;
    return 0;
  }
  for (int i = 0; i < m-1; i++) {
    if (S[i] == 'O' && S[i+1] == 'O') {
      cout << "INVALID" << endl;
      return 0;
    }
  }
  // now solve the problem.
  reverse(S.begin(), S.end());

  long long ans = -1;
  for (long long v = 8; v <= (1LL<<47); v*=2) {
    long long u = v;
    for (int i = 0; i < m; i++) {
      if (S[i] == 'O') {
        if ((u-1)%3 != 0) { u=-1; break; }
        u = (u-1)/3;
      } else if (S[i] == 'E') {
        u *= 2;
        if(u > (1LL<<62)) { u=-1; break; }
      }
    }
    if (u != -1 && (ans == -1 || ans > u)) {
      ans = u;
    }
  }
  assert(ans != -1);
  cout << ans << endl;
  return 0;
}
