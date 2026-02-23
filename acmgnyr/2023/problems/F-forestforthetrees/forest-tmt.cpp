#include <bits/stdc++.h>
using namespace std;

const int N = 5005;
int x[N], y[N];
int xx[N], yy[N];
int px[N], py[N];
class Segment{
  public:
  int xl, xr, xmid;
  Segment *lptr, *midptr, *rptr;
  vector<int> data;
  Segment() {lptr=midptr=rptr=nullptr;}
};

Segment* build(vector<int> data) {
  Segment* ret = new Segment();
  sort(data.begin(), data.end(), [&](int i, int j) {
    if(xx[i]!=xx[j]) return xx[i] < xx[j];
    if(yy[i]!=yy[j]) return yy[i] < yy[j];
    return i<j;
  });
  
  int len=data.size();
  ret->data = data; // copy
  ret->xl = xx[data[0]];
  ret->xr = xx[data.back()];
  ret->xmid = xx[data[len/2]];

  sort(ret->data.begin(), ret->data.end(), [&](int i, int j) {
    if (yy[i]!=yy[j]) return yy[i] < yy[j];
    if (xx[i]!=xx[j]) return xx[i] < xx[j];
    return i<j;
  });
  if (ret->xl == ret->xr) return ret;
  int mid=len/2;
  while(mid>0 && xx[data[mid-1]]==xx[data[mid]]) --mid;
  if(mid>0) {
    ret->lptr = build(vector<int>(data.begin(), data.begin() + mid));
  }
  int midl = mid;
  mid=len/2;
  while(mid+1<len && xx[data[mid+1]]==xx[data[mid]]) ++mid;
  if(mid+1<len) {
    ret->rptr = build(vector<int>(data.begin() + mid + 1, data.end()));
  }
  int midr = mid;
  ret->midptr = build(vector<int>(data.begin() + midl, data.begin() + midr + 1));
  return ret;
}

int nt, ns, rmax;
const int OFFSET=524288;
const long long MASK=(1LL<<20)-1;
unordered_set<long long> dict;
unordered_set<long long> alltrees;
long long enc(int vx, int vy) {
  return (vx+OFFSET)*(1LL<<20)+(vy+OFFSET);
}
void dec(int &vx, int &vy, long long val) {
  vx=(val>>20) - OFFSET;
  vy=(val&MASK) - OFFSET;
}
int mysearch(const vector<int>& data, int yl) {
  int l=0,r=(int)data.size()-1, ans=(int)data.size();
  while(l<=r) {
    int m=(l+r)/2;
    if(yy[data[m]] < yl) { l=m+1; }
    else { ans=m; r=m-1; }
  }
  return ans;
}
const int infcnt=100000;
int xl, xr, yl, yr, cx, cy;
int ask(Segment *node) {
  int ret=0;  
  if (node->xl > xr || node->xr < xl) return 0;
  if (node->xl >= xl && node->xr <= xr) {
    int idx = mysearch(node->data, yl);
    while(idx<node->data.size() && yy[node->data[idx]] <= yr) {
      int c = node->data[idx];
      if(dict.find(enc(x[c]-cx, y[c]-cy)) == dict.end()) {
        return infcnt;
      }
      ++ret;
      ++idx;
    }
    return ret;
  }
  if(node->lptr) ret += ask(node->lptr);
  if(node->midptr) ret += ask(node->midptr);
  if(node->rptr) ret += ask(node->rptr);
  int retv= min(ret, infcnt);
  return retv;
}
bool query(Segment *root, int _cx, int _cy) {
  cx=_cx;cy=_cy;
  xl=cx-cy-rmax;
  xr=cx-cy+rmax;
  yl=cx+cy-rmax;
  yr=cx+cy+rmax;
  int ret = ask(root);
  return (ret == ns) ;
}

int bestcnt=0, bestx, besty;
void solve(Segment* root) {
  dict.clear();
  for(int i=0;i<ns;i++) {
    dict.insert(enc(px[i], py[i]));
  }
  unordered_map<long long, int> cnt;
  for(int i=0;i<nt;i++)
    for(int j=0;j<ns;j++) {
      cnt[enc(x[i]-px[j], y[i]-py[j])] += 1;
    }
  
  for(auto& it : cnt) {
    int cx, cy;
    dec(cx, cy, it.first);
    // skip same location with tree.
    if (alltrees.find(enc(cx, cy)) != alltrees.end()) continue;
    int cnts = it.second;
    assert(cnts <= ns);
    if (cnts == ns) {
      bool ok = query(root, cx, cy);
      if (ok) {
        ++bestcnt;
        bestx=cx;
        besty=cy;
      }
    }
  }
}

int main() {
  scanf("%d%d%d", &nt, &ns, &rmax);
  for(int i=0;i<nt;i++) {
    scanf("%d%d", &x[i], &y[i]);
    alltrees.insert(enc(x[i], y[i]));
    xx[i]=x[i]-y[i];
    yy[i]=x[i]+y[i];
  }
  for(int i=0;i<ns;i++) {
    scanf("%d%d", &px[i], &py[i]);
  }
  vector<int> all;
  for(int i=0;i<nt;i++) all.push_back(i);
  Segment* root = build(all);
  for(int _=0;_<4;_++) {
    solve(root);
    for(int i=0;i<ns;i++) {
      swap(px[i], py[i]);
      py[i]=-py[i];
    }
  }
  if(bestcnt==0) puts("Impossible");
  else if(bestcnt>1) puts("Ambiguous");
  else printf("%d %d\n", bestx, besty);
  return 0;
}
