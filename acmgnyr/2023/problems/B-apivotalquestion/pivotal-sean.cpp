#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int my_min(int a, int b){
  if(a < b)
    return a;
  else
    return b;
}

int main(){
  int n;
  cin >> n;
  vector<int> v(n);
  for(int i = 0; i < n; i++)
    cin >> v[i];
  
  vector<bool> ok_left(n, false);
  int max = v[0];
  ok_left[0] = true;
  for(int i= 1; i < n; i++){
    if(v[i] > max){
      ok_left[i] = true;
      max = v[i];
    }
  }

  int min = v[n-1];

  vector<int> pivots;
  if(ok_left[n-1])
    pivots.push_back(v[n-1]);
  
  for(int i = n-2; i >= 0; i--){
    if(v[i] < min){
      min = v[i];
      if(ok_left[i])
	pivots.push_back(v[i]);
    }
  }

  reverse(pivots.begin(), pivots.end());
  int end = my_min(pivots.size(), 100);
  cout << pivots.size() << " ";
    for(int i = 0; i < end; i++)
    cout << pivots[i] << " ";
  cout << endl;

  return 0;
}

    
    
    
