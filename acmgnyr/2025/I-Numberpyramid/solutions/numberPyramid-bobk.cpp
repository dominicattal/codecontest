#include <cstdint>
#include <iostream>

using namespace std;

#define REPI(ctr,start,limit) for (uint32_t ctr=(start);(ctr)<(limit);(ctr)++)

constexpr uint32_t
    MAX_SIZE = 100;

int32_t
    card[MAX_SIZE+2][MAX_SIZE+2];

int main() {
    uint32_t
        size;
    bool
        changed;

    REPI(i,0,MAX_SIZE+2)
        REPI(j,0,MAX_SIZE+2)
            card[i][j] = 100;

    cin >> size;

    REPI(i,0,size)
        REPI(j,0,i+1)
            cin >> card[i+1][j+1];

    do {
        changed = false;

        REPI(i,0,size) {
            REPI(j,0,i+1) {
                if (card[i+1][j+1] == 100) {
                    // entry is lower right corner
                    if (card[i][j] != 100 && card[i+1][j] != 100) {
                        card[i+1][j+1] = card[i][j] - card[i+1][j];
                        changed = true;
                    }
                    // entry is lower left corner
                    if (card[i][j+1] != 100 && card[i+1][j+2] != 100) {
                        card[i+1][j+1] = card[i][j+1] - card[i+1][j+2];
                        changed = true;
                    }
                    // entry is top
                    if (card[i+2][j] != 100 && card[i+2][j+1] != 100) {
                        card[i+1][j+1] = card[i+2][j+1] + card[i+2][j+2];
                        changed = true;
                    }
                }
            }
        }
    } while (changed);

    REPI(i,0,size) {
        REPI(j,0,i+1) {
            if (card[i+1][j+1] == 100) {
                cout << "unsolvable" << endl;
                return 0;
            }
            if (i != size - 1 && card[i+1][j+1] != card[i+2][j+1] + card[i+2][j+2]) {
                cout << "contradiction" << endl;
                return 0;
            }
        }
    }

    cout << "solvable" << endl;
    REPI(i,0,size) {
        cout << card[i+1][1];
        REPI(j,1,i+1) {
            cout << ' ' << card[i+1][j+1];
        }
        cout << endl;
    }

    return 0;
}