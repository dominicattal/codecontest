#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

typedef unsigned long DWORD;
typedef unsigned char BYTE;
#ifdef WIN32
typedef unsigned __int64 DDWORD;
#else
typedef unsigned long long DDWORD;
#endif

DDWORD fact, denom, val, sub;
int factPow;
char inSeq[256];

int ParseEO(char *inp)
{
	int ind, cnt, lastOdd;
	char c;
	factPow = 0;
	ind = cnt = lastOdd = 0;
	c = inp[ind];
	fact = denom = 1; sub = 0;
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
	for(ind = cnt-1; ind >= 0 ; ind--) {
		if(inSeq[ind] == 1) {
			sub += denom;
			denom *= 3;
		} else {
			fact *= 2;
			sub *= 2;
			factPow++;
		}
	}
	return cnt;
}

int FindPow()
{
	DDWORD pow, rem, s;
	int cnt = factPow;
	pow = rem = fact;
	s = sub % denom;
	while (pow <= sub) {
		pow *= 2;
		rem = (rem*2) % denom;
		cnt++;
	}
	while ((cnt < 64) && ((rem - s + denom) % denom != 0)) {
		pow *= 2;
		rem = (rem*2) % denom;
		cnt++;
	}
	if(cnt >= 64) {
		return -1;
	}
	val = (pow - sub)/denom;
	return 0;
}

DDWORD col(DDWORD in) {
	if((in & 1) != 0) {
		return (3*in + 1);
	} else {
		return (in/2);
	}
}

int Validate(DDWORD val, int cnt)
{
	int i;
	for(i = 0; i < cnt ; i++) {
		if((val & 1) != 0) {
			if(inSeq[i] != 1) {
				return -1;
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
	if(FindPow() != 0) {
		return -3;
	}
	if((ret = Validate(val, ret)) != 0) {
		fprintf(stderr, "invalid resutl %llu ret %d\n", val, ret);
		return -3;
	}

	printf("%llu\n", val);
	return 0;
}
