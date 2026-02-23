#include <iostream>
#include <vector>
#include <cstring>
using namespace std;

const int MAXS = 200;
int rows, cols, n, rs, cs;
char p[MAXS+1];
int dr[4] = {0,-1,1,0}; // E,N,S,W row change
int dc[4] = {1,0,0,-1}; // E,N,S,W col change
int *ans;
vector<vector<char>> grid;

// Manhattan distance from (r,c) to (rs,cs). Since we can't
// retrace any part of the path from (rs,cs) to (r,s), we must
// add 2 whenever (rs,cs) is in the same row or column as (r,c),
// but not both, and the direction of travel is along that row or column.
int md(int dir, int r, int c) {
  int d = abs(r-rs)+abs(c-cs);
  if ((dir==0 && r==rs && c > cs) ||
      (dir==1 && c==cs && r < rs) ||
      (dir==2 && c==cs && r > rs) ||
      (dir==3 && r==rs && c < cs)) d += 2;
  return d;
}

// Most of the work done here
bool solve(int i, int r, int c, int dir) {
  // Where are we trying to move to?
  int r1 = r+dr[dir];
  int c1 = c+dc[dir];

  // Is there a vacant cell there? (does not apply to last move)
  if (i < n-1 && grid[r1][c1] != ' ') {
    return false;
  }
 
  // Is the next cell too far away from the start cell?
  // Number of moves left, including the one we're trying to make, = n-1-i

  if (md(dir,r1,c1) > n-1-i) {
    return false;
  }

  // Is this the last move?
  if (i == n-1) {
    // Are we back at the beginning? 
    if (r1 != rs || c1 != cs) {  // do we need this test?
      return false;
    }

    // Yes, we're back to p[0]. There must be a pearl here.
    // Make sure it obeys the pearl rules.
    if (p[0] == 'B') {
      if (dir == ans[0]) {
        // no corner at a black pearl--fail
        return false;
      }
      if (dir != ans[n-2] || ans[0] != ans[1]) {
        // corner beside black pearl--fail
        return false;
      }
    }

    if (p[0] == 'W') {
      if (dir != ans[0]) {
        // corner in white pearl--fail
        return false;
      }
      if (ans[0]==ans[1] && ans[n-2]==dir) {
        // no corner on either side of white pearl--fail
        return false;
      }
    }

      if (p[n-1] == 'B') {
        if (dir != ans[0]) {
          // corner next to black pearl--fail
          return false;
        }
      }

      if (p[n-1] == 'W') {
        if (ans[n-2] == ans[n-3] && dir == ans[0]) {
          // no corner next to white pearl--fail
          return false;
        }
      }

    // Success! We've solved the puzzle
    ans[i] = dir;
    for (int j = 0; j < n; j++) {
      cout << "ENSW"[ans[j]];
    }
    cout << endl;
    return true;
  } 


  // This is NOT the last move, so continue. Some directions of travel may
  // be excluded, depending on whether there's a pearl at p[i].

  // Are we placing a black pearl? If so, can't be preceded by a corner
  if (i > 0 && p[i+1]=='B' && dir != ans[i-1]) {
    return false;
  }

  // Placement okay so far--continue onward
  ans[i] = dir;

  // Place the pearl or dot:
  grid[r1][c1] = p[i+1];

  // Now look at each possible direction for placement of next item:
  for (int d = 0; d < 4; d++) {
    // Can't go backwards:
    if (d == 3-dir) {
      continue;
    }

    // Can't go perpendicular from a white pearl on an edge:
    if (p[i+1]=='W' && (r1==1 || r1==rows) && (d==1 || d==2)) {
      continue;
    }
    if (p[i+1]=='W' && (c1==1 || c1==cols) && (d==0 || d==3)) {
      continue;
    }

    // If we just placed two white pearls in a row, we must change direction:
    if (i > 0 && p[i]=='W' && p[i-1]=='W' && d==dir) {
      continue;
    }

    // If we just placed a black pearl we must change direction:
    if (p[i+1]=='B' && d == dir) {
      continue;
    }

    // If previous move was a black pearl, we must NOT change direction:
    if (p[i]=='B' && d != dir) {
      continue;
    }

    // If the newly-placed item is white then we have to continue in the
    // same direction from which we arrived. 
    if (p[i+1]=='W' && d != dir) {
      continue;
    }
    
    // if the previously-placed item was 'W', see if it was preceded by a
    // corner; if not, then we MUST have a corner here. 
    if (i > 1 && p[i]=='W' && ans[i-1]==ans[i-2] && d==dir) {
      continue;
    }
    
    if (solve(i+1,r1,c1,d)) return true;
  }
  grid[r1][c1] = ' ';
  ans[i] = 4;
  return false;
}

int main() {
  cin >> n >> rows >> cols;
  cin >> p;

  if (strlen(p)%2 != 0) {
    cout << "impossible" << endl;
    exit(0);
  }
  if (strstr(p,"BB")) {
    cout << "impossible" << endl;
    exit(0);
  }
  if (p[0]=='B' && p[n-1]=='B') {
    cout << "impossible" << endl;
    exit(0);
  }
  if (strstr(p,"WWW")) {
    cout << "impossible" << endl;
    exit(0);
  }

  char z[5] = {p[n-2],p[n-1],p[0],p[1],0};  
  if (strstr(z,"WWW")) {
    cout << "impossible" << endl;
    exit(0);
  }
  cin >> rs >> cs;
  ans = new int[n];
  ans[n-2]=ans[n-1]=4; // dummy directions so we can check placement of

  grid.resize(rows+2,vector<char>(cols+2,' '));

  for (int i = 0; i < cols+2; i++) {
    grid[0][i] = grid[rows+1][i] = 'X';
  }
  for (int i = 1; i < rows+1; i++) {
    grid[i][0] = grid[i][cols+1] = 'X';
  }
  grid[rs][cs] = p[0];
  bool done=false;
  for (int d = 0; d < 4; d++) {
    // we can save a little time if we check for 'W' along the border:
    if (p[0]=='W' && (rs==1 || rs==rows) && (d==1 || d==2)) {
      continue;
    }
    if (p[0]=='W' && (cs==1 || cs==cols) && (d==0 || d==3)) {
      continue;
    }
    if (solve(0,rs,cs,d)) { done=true; break;}
  }
  if (!done) cout<< "impossible" << endl;
}
