#include <string>
#include <vector>
#include <iostream>

using namespace std;

bool Valid(string s){
  // You're invalid if you have the wrong number of digits
  if(s.size() < 10 || s.size() > 13)
    return false;
  // Let's look at hyphens
  int hyphen_count = 0;
  // you're invalid if you start or end with a hyphen
  if(s[0] == '-' || s[s.size()-1] == '-')
     return false;
  int total = 0;
  int total_multiplier = 10;
  for(int i = 0; i < s.size()-1; i++){
    if(s[i] == '-'){
      hyphen_count++;
      if(s[i+1] == '-') // you're invalid if you have 2 consecutive hyphens
	return false;
    }
    else{
      total = total + (s[i] - '0')* total_multiplier;
      total_multiplier--;
    }
  }
  // you're invalid if size - hyphen_count is not 10
  if(s.size()-hyphen_count != 10)
    return false;

  // you're invalid if size is 13, and position 11 is not a hyphen
  if(s.size() == 13 && s[11] != '-')
    return false;
  
  // you're invalid if an 'X' is anywhere but the end
  int x_pos = s.find('X');
  if(x_pos != s.size()-1 && x_pos != string::npos)
    return false;

  
  // you're invalid if your checksum is wrong
  char checksum_char = s[s.size()-1];
  int checksum_num;
  if(checksum_char == 'X')
    total = total + 10;
  else
    total = total +  checksum_char - '0';
  if(total%11 != 0)
    return false;

  // I think that's it?
  return true;
}

int Checksum(string s){
  int multiplier = 3;  // because 978 is 3 digits, we start at digit 4
  int total = 9*1+7*3+8*1;
  for(int i = 0; i < s.size()-1; i++){
    if(s[i] != '-'){
      total = total + multiplier * (s[i]-'0');
      if(multiplier == 3)
	multiplier = 1;
      else
	multiplier = 3;
    }
  }
  return  (10-(total%10)) % 10;
}
      
     
int main(){
  int t;
  cin >> t;
  for(int i = 0; i < t; i++){
    string cur;
    cin >>cur;
    if(!Valid(cur))
      cout << "invalid" << endl;
    else{
      int c = Checksum(cur);
      cout << "978-" << cur.substr(0, cur.size()-1)<<c << endl;
    }
  }

  return 0;
}

  
