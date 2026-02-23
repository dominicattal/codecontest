#include <iostream>
#include <vector>
using namespace std;

int main(){

  int total = 0;
  for(int i = 0; i < 5; i++){
    int a;
    int l;
    cin >> a >>l;
    int kernels = a*l;
    total = total + kernels;
  }
  int average = total/5;
  int n, kwf;
  cin >> n;
  int result = average * n;
  cin >> kwf;
  result = result /kwf;
  cout << result << endl;

  return 0;
}

  
