#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

typedef unsigned long DWORD;
typedef unsigned char BYTE;
#ifndef WIN32
typedef unsigned long long DDWORD;
#else
typedef unsigned __int64 DDWORD;
#endif

DDWORD fact, denom, val, sub;
int factPow;
char inSeq[256];

int ParseEO(char *inp)
{
	int ind, cnt, lastOdd;
	char c;
	ind = cnt = lastOdd = 0;
	c = inp[ind];
	while((c != 0) &&  (c != 'E') && (c != 'O')) {
		ind++;
		c = inp[ind];
	}
	if(c == 0) {
		return -1;
	}
	while((c == 'E') || (c == 'O')) {
		if(c == 'E') {
			inSeq[cnt++] = 2;
			lastOdd = 0;
		} else {
			if(lastOdd) {	// two odds in a row
				return -4;
			}
			inSeq[cnt++] = 1;
			lastOdd = 1;
		}
		ind++;
		c = inp[ind];
	}
	if(( cnt > 0) && (inSeq[cnt-1] != 1)) { // last code must be odd
		return -2;
	}
	// if there are at least 2 codes 2nd from last must be even
	if((cnt > 1) && (inSeq[cnt-2] != 2)) {
		return -3;
	}
	return cnt;
}

DDWORD lim = 0x5555555555555555;
int Validate(DDWORD val, int cnt)
{
	int i;
	for(i = 0; i < cnt ; i++) {
		if((val & 1) != 0) {
			if(inSeq[i] != 1) {
				return -1;
			}
			if(val >= lim) {
				return -4;
			}
			val = 3*val + 1;
		} else {
			if(inSeq[i] != 2) {
				return -2;
			}
			val = val/2;
		}
	}
	while(val > 1) {
		if((val & 1) != 0) {
			return -3;
		}
		val = val/2;
	}
	return 0;
}

DDWORD test[64];
DDWORD sizelim = 0x800000000000;
int FindSmallestStart(int cnt)
{
	int i, j, zerocnt;
	DDWORD tval;
	tval = 8;
	test[0] = test[1] = test[2] = test[63] = 0;
	for(i = 3; i < 63 ; i++ ) {
		test[i] = tval;
		tval *= 2;
	}
	zerocnt = 4;
	for(j = cnt - 1; (j >= 0) && (zerocnt < 64) ; j--) {
		if(inSeq[j] == 1) {
			for(i = 3; i < 63 ; i++) {
				if(test[i] != 0) {
					if((test[i] >= lim) || (((test[i] -1) % 3) != 0)) {
						test[i] = 0;
						zerocnt++;
					} else {
						test[i] = ((test[i] -1)/3);
					}
				}
			}
		} else {
			for(i = 3; i < 63 ; i++) {
				if(test[i] != 0) {
					test[i] = (test[i]*2);
				}
			}
		}
	}
	if(zerocnt == 64) {
		return -1;
	}
	for(i = 3; i < 63; i++) {
		if(test[i] != 0) {
			if(test[i] < sizelim) {
				val = test[i];
				return 0;
			} else {
				return -2;
			}
		}
	}
	return -3;
}

char inbuf[256];
int main()
{
	int ret;
	if(fgets(&(inbuf[0]), 255, stdin) == NULL) {
		fprintf(stderr, "read failed on input\n");
		return -1;
	}
	if((ret = ParseEO(&(inbuf[0]))) < 0) {
		printf("INVALID\n");
		return 0;
	}
	if(FindSmallestStart(ret) != 0) {
		printf("INVALID\n");
		return 0;
	}
	if((ret = Validate(val, ret)) != 0) {
		fprintf(stderr, "invalid result %llu ret %d\n", val, ret);
		return -3;
	}

#ifndef WIN32
	printf("%llu\n", val);
#else
	printf("%I64u\n", val);
#endif
	return 0;
}
