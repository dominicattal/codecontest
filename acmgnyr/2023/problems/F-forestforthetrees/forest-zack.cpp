// Zack tries to remember C++.
#include <unordered_set>
#include <iostream>
#include <utility>
#define MAXW 200000000
#define HALFW 100000000
using namespace std;

typedef unordered_set<long> treeset;

long loc(long px, long py) {
  return MAXW*(py+HALFW)+(px+HALFW);
}

long txl(long t) {
  return t%MAXW - HALFW;
}

long tyl(long t) {
  return t/MAXW - HALFW;
}

bool checkpos(long px, long py, treeset found, treeset trees, int maxr) {
  long l = loc(px,py);
  for (auto& t : trees) {
    if ((found.count(t) == 0) && (abs(px-txl(t))+abs(py-tyl(t)) <= maxr)) {
      //cout << "missing" << t << endl;
      return false;
    }
  }
  return true;
}

int main(int argc, char** argv) {
  int nt, ns, maxr;
  cin >> nt >> ns >> maxr;
  treeset trees;
  int tx, ty;
  for (int tn = 0; tn < nt; tn++) {
    cin >> tx >> ty;
    trees.insert(loc(tx,ty));
  }
  //cout << trees.size() << "trees";
  // sensor-x sensor-y (sideways amt, forward amt)
  int sens[ns][2];
  // int sx,sy;
  for (int s = 0; s < ns; s++) {
    cin >> sens[s][0] >> sens[s][1];
    if (sens[s][0] == 0 && sens[s][1] == 0) {
      cout << "Impossible" << endl;
      exit(0);
    }
  }
  bool hasplace = false;
  pair<int,int> place;
  
  // pointing +x
  for (auto& t : trees) {
    long px = txl(t)-sens[0][1];
    long py = tyl(t)+sens[0][0];
    // cout << "pos " << px << " " << py << endl;
    bool fail = false;
    treeset found;
    found.insert(t);
    for (int s = 1; s < ns; s++) {
      long tx = px+sens[s][1];
      long ty = py-sens[s][0];
      // cout << tx << " " << ty << endl;
      long tloc = loc(tx,ty);
      if (trees.count(tloc) > 0) {
        found.insert(tloc);
      } else {
        // cout << "not " << tx << " " << ty << endl;
        fail = true;
        break;
      }
    }
    if (!fail && checkpos(px,py,found,trees,maxr)) {
      //cout << "place " << px << " " << py << " +x" << endl;
      if (!hasplace) {
        place = make_pair(px,py);
        hasplace = true;
      }
      else {
        cout << "Ambiguous" << endl;
        exit(0);
      }
    }
  }

  // pointing +y
  for (auto& t : trees) {
    long px = txl(t)-sens[0][0];
    long py = tyl(t)-sens[0][1];
    bool fail = false;
    treeset found;
    found.insert(t);
    for (int s = 1; s < ns; s++) {
      long tx = px+sens[s][0];
      long ty = py+sens[s][1];
      long tloc = loc(tx,ty);
      if (trees.count(tloc) > 0) {
        found.insert(tloc);
      } else {
        fail = true;
        break;
      }
    }
    if (!fail && checkpos(px,py,found,trees,maxr)) {
      //cout << "place " << px << " " << py << " +y" << endl;
      if (!hasplace) {
        place = make_pair(px,py);
        hasplace = true;
      }
      else {
        cout << "Ambiguous" << endl;
        exit(0);
      }
    }
  }

  // pointing -x
  for (auto& t : trees) {
    long px = txl(t)+sens[0][1];
    long py = tyl(t)-sens[0][0];
    //cout << "pos " << px << " " << py << endl;
    bool fail = false;
    treeset found;
    found.insert(t);
    for (int s = 1; s < ns; s++) {
      long tx = px-sens[s][1];
      long ty = py+sens[s][0];
      long tloc = loc(tx,ty);
      if (trees.count(tloc) > 0) {
        found.insert(tloc);
      } else {
        fail = true;
        break;
      }
    }
    if (!fail && checkpos(px,py,found,trees,maxr)) {
      //cout << "place " << px << " " << py << " -x" << endl;
      if (!hasplace) {
        place = make_pair(px,py);
        hasplace = true;
      }
      else {
        cout << "Ambiguous" << endl;
        exit(0);
      }
    }
  }

  // pointing -y
  for (auto& t : trees) {
    long px = txl(t)+sens[0][0];
    long py = tyl(t)+sens[0][1];
    bool fail = false;
    treeset found;
    found.insert(t);
    for (int s = 1; s < ns; s++) {
      long tx = px-sens[s][0];
      long ty = py-sens[s][1];
      long tloc = loc(tx,ty);
      if (trees.count(tloc) > 0) {
        found.insert(tloc);
      } else {
        fail = true;
        break;
      }
    }
    if (!fail && checkpos(px,py,found,trees,maxr)) {
      //cout << "place " << px << " " << py << " -y" << endl;
      if (!hasplace) {
        place = make_pair(px,py);
        hasplace = true;
      }
      else {
        cout << "Ambiguous" << endl;
        exit(0);
      }
    }
  }

  if (!hasplace)
    cout << "Impossible" << endl;
  else
    cout << place.first << " " << place.second << endl;
}

  
