/*
 * Sum of Remainders
 * Greater NY Regional 2022
 * Fred Pickel
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef unsigned long DWORD;
typedef unsigned char BYTE;

char inbuf[256];
int nVals, curInd, curVal, kCnt, kTot, kRem;
int Vals[10];
int kVals[10];
int GetRem()
{
	int i, rem;
	rem = curVal;
	for(i = 0; i < kCnt; i++) {
		rem -= curInd % kVals[i];
	}
	return rem;
}

int main()
{
	int i, expcnt, remcnt, ret, rem, newrem;
	if(fgets(&(inbuf[0]), 255, stdin) == NULL) {
		fprintf(stderr, "read failed on value count\n");
		return -1;
	}
	if((ret = sscanf(&(inbuf[0]), "%d", &nVals)) != 1) {
		fprintf(stderr, "scan of start pos ret %d\n", ret);
		return -2;
	}
	remcnt = nVals;
	curInd = 1;
	kCnt = 0;
	while (remcnt > 0) {
		if(fgets(&(inbuf[0]), 255, stdin) == NULL) {
			fprintf(stderr, "read failed on vals @ %d\n", remcnt);
			return -3;
		}
		if(remcnt > 10) expcnt = 10;
		else expcnt = remcnt;
		if((ret = sscanf(&(inbuf[0]), "%d %d %d %d %d %d %d %d %d %d",
			&(Vals[0]), &(Vals[1]), &(Vals[2]), &(Vals[3]), &(Vals[4]), 
			&(Vals[5]), &(Vals[6]), &(Vals[7]), &(Vals[8]), &(Vals[9]))) != expcnt) {
			fprintf(stderr, "scan of vals ret %d != expected %d\n", ret, expcnt);
			return -4;
		}
		remcnt -= expcnt;
		for(i = 0; i < expcnt; i++, curInd++) {
			curVal = Vals[i];
			if(curInd == 1) {
				kTot = kRem = curVal;
			} else {
				rem = GetRem();
				if((rem % curInd) != 0) {
					fprintf(stderr, "Bad rem %d at ind %d val %d\n",
						rem, curInd, curVal);
					return -5;
				}
				newrem = rem/curInd;
				if(newrem < kRem) {
					while (newrem < kRem) {
						kVals[kCnt++] = curInd;
						kRem--;
					}
				}
				if(kRem == 0) {
					remcnt = 0;
					break;
				}
			}
		}
	}
	printf("%d", kTot);
	for(i = 0; i < kTot; i++) {
		printf(" %d", kVals[i]);
	}
	printf("\n");
	return 0;
}
