// by tmt514
#include <bits/stdc++.h>
using namespace std;

int main() {
  int A, L, total = 0;
  for (int i = 0; i < 5; i++) {
    cin >> A >> L;
    total += A * L;
  }
  total /= 5;
  int N, KWF;
  cin >> N >> KWF;
  cout << total * N / KWF << endl;
  return 0;
}
