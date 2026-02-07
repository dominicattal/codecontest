#include <iostream>

using namespace std;

int main(){
  int num_s = 0;
  int num_m = 0;
  int num_l = 0;

  int n;
  cin >> n;
  for(int i = 0; i < n; i++){
    char type;
    int amt;
    cin >> type >> amt;
    if(type == 'S')
      num_s = num_s + amt;
    if(type == 'M')
      num_m = num_m + amt;
    if(type == 'L')
      num_l = num_l + amt;
  }

  int s_pizza = num_s/6;
  if(num_s % 6 != 0)
    s_pizza++;
  int m_pizza = num_m/8;
  if(num_m % 8 != 0)
    m_pizza++;

  int l_pizza = num_l/12;
  if(num_l % 12 != 0)
    l_pizza++;

  cout << s_pizza+m_pizza+l_pizza << endl;
  return 0;
}
