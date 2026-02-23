#include <bits/stdc++.h>
using namespace std;

vector<string> code = {".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---",
"-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", 
".--", "-..-", "-.--", "--..", "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----."};
int toChar(string S) {
  for(int i=0;i<26;i++) if(code[i]==S) return i+'A';
  for(int i=26;i<36;i++) if(code[i]==S) return i+'0'-26;
  return -1;
}
string toMorse(string S) {
  string ret="";
  for(char x : S) {
    if(x>='A' && x<='Z') ret += code[x-'A'];
    else ret += code[x-'0'+26];
  }
  return ret;
}

void update(string &X, string P) {
  vector<string> dp(P.size()+1, string(100, 'x'));
  dp[0] = "";
  for(int i=0;i<P.size();i++) {
    for(int j=1;j<=5 && i+j<=P.size();j++) {
      int c = toChar(P.substr(i, j));
      if (c != -1 && dp[i+j].size() > dp[i].size() + 1) {
        dp[i+j] = dp[i] + (char)c;
      }
    }
  }
  if(X.size() > dp[P.size()].size()) X = dp[P.size()];
}

int main() {
  string T;
  getline(cin, T);
  string S = "";
  for(char x : T) if ((x >= 'A' && x <= 'Z') || (x >= '0' && x <= '9')) S += x;

  string C = toMorse(S);
  int n = C.size();
  string X(100, 'n');
  for(int i=0;i<n;i++) {
    string R = C.substr(i);
    string P = C.substr(0, i);
    reverse(R.begin(), R.end());
    reverse(P.begin(), P.end());
    if (C.substr(i) == R) {
      update(X, P);
    }
  }

  printf("%d", (int)X.size());
  if(X.size()) printf(" %s", X.c_str());
  puts("");
  return 0;
}