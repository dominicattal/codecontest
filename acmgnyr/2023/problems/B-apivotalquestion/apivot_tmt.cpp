#include <bits/stdc++.h>
using namespace std;

int main() {
  int n;
  scanf("%d", &n);

  vector<int> a(n), b(n, 0);
  for(int i = 0; i < n; i++) scanf("%d", &a[i]);
  for(int i = 0, curmax = 0; i < n; i++) {
    b[i] += (a[i] >= curmax);
    curmax = max(curmax, a[i]);
  }
  for(int i = n-1, curmin = INT_MAX; i >= 0; i--) {
    b[i] += (i == n-1 || a[i] < curmin);
    curmin = min(curmin, a[i]);
  }
  int ans = 0;
  vector<int> ret;
  for(int i=0;i<n;i++) {
    if (b[i] == 2) {
      ++ans;
      if (ret.size() < 100) ret.push_back(a[i]);
    }
  }
  printf("%d", ans);
  for (int x : ret) printf(" %d", x);
  puts("");
  return 0;
}
