#include <cstdint>
#include <iostream>

using namespace std;

int main() {
    uint32_t
        n,
        nS=0,nM=0,nL=0,
        leftover;
    char
        size;

    cin >> n;

    for (uint32_t i = 0; i < n; i++) {
        cin >> size >> leftover;

        if (size == 'S')
            nS += leftover;
        else if (size == 'M')
            nM += leftover;
        else if (size == 'L')
            nL += leftover;
    }

    leftover = (nS + 5) / 6 + (nM + 7) / 8 + (nL + 11) / 12;

    cout << leftover << endl;

    return 0;
}