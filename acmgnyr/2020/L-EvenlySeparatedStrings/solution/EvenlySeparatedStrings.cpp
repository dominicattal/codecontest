/**
 * @file EvenlySeparatedStrings.cpp
 * @author Changyu Zhu (changyuzhu99@gmail.com)
 * @brief Brute force short strings. Reject long strings.
 * @date 2021-01-27
 * 
 * @copyright Copyright (c) 2021
 *
 * @description Notice that if any letter appears more than twice, then the string
 * cannot be evenly separated: Pick 3 arbitrary indices of the letter and let the 
 * indices be i < j < k. Then the numbers of characters between those positions
 * are (j-i-1), (k-j-1), (k-i-1). These three numbers cannot be all even: If the
 * first two are even, then the sum of them would still be even, i.e.,
 * (j-i-1)+(k-j-1)=k-i-2 is even, and therefore k-i-1 must be odd, contradiction.
 * Therefore, we reject immediately if the length of the string is > 52.
 * When the length of the string is <= 52, we can simply brute force to find out the answer.
 */
#include <string>
#include <iostream>
using namespace std;

int main() {
  string s;
  cin >> s;
  if (s.length() > 52) {
    puts("NO");
    return 0;
  }
  for (int i = 0; i < s.length(); i++) {
    for (int j = i+1; j < s.length(); j++) {
      if (s[i] == s[j] && ((j-i-1) & 1)) {
        puts("NO");
        return 0;
      }
    }
  }
  puts("YES");
  return 0;
}