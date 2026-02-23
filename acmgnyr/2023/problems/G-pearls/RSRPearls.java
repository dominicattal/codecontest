// Slow, kludgy, painstaking backtracking

import java.util.*;

public class RSRPearls {
  public static int[] dr, dc;
  public static String ps;
  public static char[] p;
  public static int[] ans;
  public static char[][] grid;
  public static int rows, cols,n,rs,cs;

  public static void main(String[] args) {
    Scanner in = new Scanner(System.in);
    n = in.nextInt();
    rows = in.nextInt();
    cols = in.nextInt();
    ps = in.next();
    // before doing anything else, check some basic stuff:

    if (ps.length()%2 != 0) {
      System.out.println("impossible");
      System.exit(0);
    }
//System.out.println("'"+ps+ps.substring(0,2)+"'");
    if ((ps+ps.substring(0,2)).contains("WWW")) {
      System.out.println("impossible");
      System.exit(0);
    }
    if ((ps+ps.substring(0,2)).contains("BB")) {
      System.out.println("impossible");
      System.exit(0);
    }

    p = ps.toCharArray();
    ans = new int[n];
    ans[n-2]=ans[n-1]=4; // dummy directions so we can check placement of
                         // first char after an initial 'W' pearl
    dr = new int[]{0,-1,1,0}; // E,N,S,W row change
    dc = new int[]{1,0,0,-1}; // E,N,S,W col change
    grid = new char[rows+2][cols+2];

    // put a border around the grid so we can detect the edges

    Arrays.fill(grid[rows+1],'X');
    for (int i = 1; i <= rows; i++) {
      Arrays.fill(grid[i],' ');
      grid[i][0] = grid[i][cols+1] = 'X';
    }

    // Start position:
    rs = in.nextInt();
    cs = in.nextInt();

    grid[rs][cs] = p[0];
    boolean done=false;
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
    if (!done) System.out.println("impossible");
  }


/* ONLY USED FOR DEBUGGING
  public static void show() {
    for (int i = 0; i < n; i++) {
      System.out.print("ENSWX".charAt(ans[i]));
    }
    System.out.println();
    for (int r = 0; r < rows+2; r++) {
      for (int c = 0; c < cols+2; c++) {
        System.out.print(grid[r][c]);
      }
      System.out.println();
    }
  }
*/

  // Manhattan distance from (r,c) to (rs,cs). Since we can't
  // retrace any part of the path from (rs,cs) to (r,s), we must
  // add 2 whenever (rs,cs) is in the same row or column as (r,c),
  // but not both, and the direction of travel is along that row or column.
  public static int md(int dir, int r, int c) {
    int d = Math.abs(r-rs)+Math.abs(c-cs);
    if ((dir==0 && r==rs && c > cs) ||
        (dir==1 && c==cs && r < rs) ||
        (dir==2 && c==cs && r > rs) ||
        (dir==3 && r==rs && c < cs)) d += 2; // minor kludge allows pruning
                                             // ever-so-slightly earlier
    return d;
  }

  // Most of the work done here. p[i] has been placed in location (r,c)
  // and we left (r,c) in direction dir = ans[i-1] (for i > 0). Let
  // (r1,c1) be the cell we arrived at.
  // We must see if it is legal to place p[i+1] in (r1,c1) and, if so,
  // determine all valid directions d in which we can move next. (Special
  // cases for i = n-1 since next move leads back to p[0] in (rs,cs).)

  public static boolean solve(int i, int r, int c, int dir) {
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
      if (r1 != rs || c1 != cs) {  // do we need this test anymore??
        return false;
      }

      // Yes, we're back to p[0] in location (rs,cs). There is a pearl here;
      // make sure it obeys the pearl rules.
      if (p[0] == 'B') {
        if (dir == ans[0]) {
          // no corner at a black pearl--fail
          return false;
        }
        if (dir != ans[n-2] || ans[0] != ans[1]) { // don't need second part?
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
        System.out.print("ENSW".charAt(ans[j]));
      }
      System.out.println();
      return true;
    } 


    // This is NOT the last move, so continue. Some directions of travel may
    // be excluded, depending on whether there's a pearl at p[i].

    // Are we placing a black pearl? If so, can't be preceded by a corner
    try {
    if (i > 0 && p[i+1]=='B' && dir != ans[i-1]) {
      return false;
    }
    } catch(ArrayIndexOutOfBoundsException e) { // don't need this any more!
      System.err.println(e);
      System.err.println("i+1="+(i+1));
      System.exit(1);
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
      if (i > 1 && p[i]=='W' && dir==ans[i-2] && d==dir) {
        continue;
      }
      
      if (solve(i+1,r1,c1,d)) return true;
    }
    grid[r1][c1] = ' ';
    ans[i] = 4;
    return false;
  }
}
