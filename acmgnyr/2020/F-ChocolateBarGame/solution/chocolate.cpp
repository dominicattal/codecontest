/**
 * @file chocolate.cpp
 * @author Changyu Zhu (changyuzhu99@gmail.com)
 * @brief Take care of 2 x 2 squares.
 * @date 2021-01-27
 * 
 * @copyright Copyright (c) 2021
 *
 * First, let.s simplify the problem by forbidding selecting 2 x 2 squares.
 * Since every other prime number is odd and the square of an odd integer is
 * still odd, both players must remove an odd number of squares from the bar
 * each turn. In this case, the winner is determined solely by the parity of
 * the number of squares left on the bar . if there are an odd number of
 * squares left, Ruby will win; Otherwise Dia wins. Their moves do not matter
 * at all.
 * 
 * Now let.s take 2 x 2 squares into consideration. By the rules of the game,
 * at most one of the players can choose a 2 x 2 square. There are two cases:
 * 1.	When in the simplified version, Ruby will win. If no 2 x 2 squares exist,
 *     or Ruby can eliminate all 2 x 2 squares by choosing a larger square that
 *     overlaps with all of them in the first turn, Ruby wins. Otherwise, Dia
 *     will be able to choose a 2 x 2 square in the second turn, change the
 *     parity of the game and win.
 * 2.	When in the simplified version, Dia will win. Then if any 2 x 2 square
 *     exists, Ruby can choose that square in the first turn and wins. Otherwise,
 *     Dia wins.
 * 
 * The first case can be implemented efficiently by recording the minimum and
 * maximum x,y coordinates of 2 x 2 squares, and checking whether a square
 * with valid coordinates exists.
 * 
 */
#include <cstring>
#include <iostream>

#define	CHOCOLATE_CHAR	'-'

using namespace std;
const int maxn = 2010, inf = 0x3f3f3f3f;
char M[maxn][maxn];
int sum[maxn][maxn];
bool isPrime[maxn];
int n;
int minx, maxx, miny, maxy;

inline int countEmpty(int x1, int y1, int x2, int y2) {
  return sum[x2][y2] - sum[x1-1][y2] - sum[x2][y1-1] + sum[x1-1][y1-1];
}

void compute2by2() {
  minx = miny = inf;
  for (int i = 1; i < n; i++) {
    for (int j = 1; j < n; j++) {
      if (countEmpty(i, j, i+1, j+1) == 4) {
        minx = min(minx, i);
        maxx = max(maxx, i + 1);
        miny = min(miny, j);
        maxy = max(maxy, j + 1);
      }
    }
  }
}

bool canEliminate2by2() {
  if (!maxx) return true;
  for (int x1 = minx; x1 <= minx+1; x1++) {
    for (int x2 = maxx-1; x2 <= maxx; x2++) {
      for (int y1 = miny; y1 <= miny+1; y1++) {
        for (int y2 = maxy-1; y2 <= maxy; y2++) {
          if (x2 - x1 != y2 - y1 || x1 > x2) continue;
          if (countEmpty(x1, y1, x2, y2) != (x2 - x1 + 1) * (x2 - x1 + 1)) continue;
          if (!isPrime[x2 - x1 + 1] || x2 - x1 == 1) continue;
          return true;
        }
      }
    }
  }
  return false;
}

int main() {
  bool firstWin;
  cin >> n;
  memset(isPrime, true, sizeof(isPrime));
  for (int i = 2; i * i <= n; i++) {
    for (int j = i * i; j <= n; j += i) {
      isPrime[j] = false;
    }
  }
  for (int i = 1; i <= n; i++) {
    cin >> (M[i] + 1);
  }
  for (int i = 1; i <= n; i++) {
    for (int j = 1; j <= n; j++) {
      sum[i][j] = sum[i-1][j] + sum[i][j-1] - sum[i-1][j-1] + (M[i][j] == CHOCOLATE_CHAR);
    }
  }

  compute2by2();
  bool flagElim = canEliminate2by2();
  if (sum[n][n] & 1) {
    firstWin = flagElim;
  } else {
    firstWin = maxy;
  }

  puts(firstWin ? "RUBY" : "DIA");
  return 0;
}
