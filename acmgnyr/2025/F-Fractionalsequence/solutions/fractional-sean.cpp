#include <iostream>

using namespace std;

long long gcd(long long a, long long b){
  long long r = a%b;
  if(r == 0)
    return b;
  else return gcd(b, r);
}

  
int main(){
  long long n;
  cin >> n;

  long long i = 1;
  long long pos = 1;
  while(pos+i <= n){
    // cout << "pos: " << pos << "i: " << i << endl;
    pos = pos + i;
    i++;
  }
  long long whole = i;
  long long num = n-pos;
  long long den = i;
  long long d = gcd(num, den);
  num = num/d;
  den = den/d;
  cout << whole << " ";
  if(num > 0)
    cout << num << "/" << den << endl;
  return 0;
}
  
