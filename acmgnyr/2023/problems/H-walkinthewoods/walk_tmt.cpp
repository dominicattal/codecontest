#include <bits/stdc++.h>
using namespace std;

int n, m;
int x[2505], y[2505];
int nbr[2505][4] = {}; // N, W, S, E
int e[10005][3] = {};

int get_direction(int i, int j) {
  if(x[i]==x[j]) {
    if(y[i] < y[j]) return 0; // North
    else return 2; // South
  } else if (y[i]==y[j]) {
    if(x[i] < x[j]) return 3; // East
    else return 1; // West
  }
  // printf("i=%d, j=%d\n", i, j);
  // return (x[i]<x[j]? 3: 1);
  assert(false);
  return -1;
}

void remove_edge(int ei) {
  int u=e[ei][0], v=e[ei][1];
  nbr[u][get_direction(u, v)] = 0;
  nbr[v][get_direction(v, u)] = 0;
}

int walkfirst(int &s, int &dir) {
  assert(nbr[s][dir] > 0);
  int ei=nbr[s][dir];
  if (--e[ei][2]==0) remove_edge(ei);
  s = (e[ei][0]^e[ei][1]^s);
  return ei;
}
int walkone(int &s, int &dir) {
  auto f=[&](int t) { return (t+4)%4; };
  int ei=0;
  int left=nbr[s][f(dir+1)]>0;
  int center=nbr[s][f(dir)]>0;
  int right=nbr[s][f(dir-1)]>0;
  if (left&&center&&right) ei = nbr[s][dir];
  else if(left) { ei=nbr[s][f(dir+1)]; dir=f(dir+1); }
  else if(center) { ei=nbr[s][f(dir)]; dir=f(dir); }
  else if(right) { ei=nbr[s][f(dir-1)]; dir=f(dir-1); }
  if (ei>0) {
    if (--e[ei][2]==0) remove_edge(ei);
    s = (e[ei][0]^e[ei][1]^s);
  }
  return ei;
}

int walk(int &s, int &dir) {
  // puts("IN");

  int used[2505][4]={};
  int eval[100005]={};
  int maxecnt[100005]={};
  int step=1;
  used[s][dir]=1;
  
  bool detected_change = false;
  while (true) {
    // printf("s=%d, dir=%d\n", s, dir);
    int ei = walkone(s, dir);
    if(ei==0) { return 0; } // the end.
    eval[step] = ei;
    if(e[ei][2]==0) {  detected_change=true; }
    if (used[s][dir] != 0) { // cycle found.
      if (detected_change) return true;
      for(int i=used[s][dir]; i <= step; i++) {
        maxecnt[eval[i]]++;
      }
      int minloop=1e9;
      for(int i=used[s][dir]; i <= step; i++) {
        minloop = min(minloop, e[eval[i]][2]/maxecnt[eval[i]]);
      }
      for(int i=used[s][dir]; i <= step; i++) {
        e[eval[i]][2] -= minloop;
        if (e[eval[i]][2] == 0) remove_edge(eval[i]);
        assert(e[eval[i]][2] >= 0);
      }
      return true;
    }
    ++step;
    used[s][dir]=step;
  }
}

int main() {
  scanf("%d%d", &n, &m);
  for(int i=1;i<=n;i++) {
    scanf("%d%d", &x[i], &y[i]);
  }
  for(int i=1;i<=m;i++) {
    scanf("%d%d%d", &e[i][0], &e[i][1], &e[i][2]);
    int u = e[i][0], v = e[i][1];
    nbr[u][get_direction(u, v)] = i;
    nbr[v][get_direction(v, u)] = i;
  }
  int s;
  char d[2];
  scanf("%d%s", &s, d);
  int dir = 0;
  if(d[0]=='N') dir=0;
  else if(d[0]=='W') dir=1;
  else if(d[0]=='S') dir=2;
  else dir=3;
  
  walkfirst(s, dir);
  while (walk(s, dir));
  printf("%d %d\n", x[s], y[s]);
  return 0;
}