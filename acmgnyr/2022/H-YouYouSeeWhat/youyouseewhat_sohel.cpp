/**
 * Problem Name: You You See What?
 * Author: Sohel Hafiz
 * Date: 2/23/2023
 * Contest: GNY Regional, 2022
*/

#include<bits/stdc++.h>
 
using namespace std;

bool sameIgnoreCase(string a, string b) {
    if (a.size() != b.size()) return false;
    for (int i  = 0; i < a.size(); i++) {
        if (tolower(a[i]) != tolower(b[i])) {
            return false;
        }
    }
    return true;
}

 
int main() {
    string s;
    cin >> s;
    assert(s.size() <= 256);
    string ss = "";
    vector<string> v;
    for (int i = 0; i < s.size(); i++) {
        if (s[i] == '!') {
            assert(ss.size() >= 1 && ss.size() <= 10);
            v.push_back(ss);
            ss = "";
        } else {
            assert(isalnum(s[i]));
            ss += s[i];
        }
    }
    assert(ss.size() >= 1 && ss.size() <= 10);
    v.push_back(ss);
    while (true) {
        bool yes = false;
        for (int i = 1; i + 2 < v.size(); i++) {
            if (sameIgnoreCase(v[i-1], v[i+1])){
                v.erase(v.begin() + i);
                v.erase(v.begin() + i);
                yes = true;
                break;
            } else if (sameIgnoreCase(v[i-1], v[i])) {
                v.erase(v.begin() + i);
                yes = true;
                break;
            } else if (sameIgnoreCase(v[i], v[i+1])) {
                v.erase(v.begin() + i + 1);
                yes = true;
                break;
            }
        }
        if (!yes) break;
    }
    cout << v[0];
    for (int i = 1; i < v.size(); i++) {
        cout << "!" << v[i];
    }
    cout << endl;
	return 0;
}
