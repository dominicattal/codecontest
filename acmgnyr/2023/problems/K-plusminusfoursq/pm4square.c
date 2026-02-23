/*
 * plus minus four squares
 * Greater NY Regional
 * Fred Pickel, Judge
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

//#define DEBUG

typedef unsigned long DWORD;
typedef unsigned char BYTE;
#ifdef WIN32
typedef unsigned __int64 DDWORD;
#else
typedef unsigned long long DDWORD;
#endif


//check if base is a perfect square and <= prev value (top)
// if so return 1 else return 0
int Find1Square(int base, int pprev, int prev, int top, int lim, char *psign)
{
	int atop;
	atop = (int)(sqrt((double)base));
	if(base == 0) {
#ifdef DEBUG
		printf("%d %c %d %c %d + 0\n",
			pprev*pprev, psign[0], prev*prev, psign[1], top*top);
#endif
		return 1;
	}
	if((atop <= lim) && (atop*atop == base)) {
#ifdef DEBUG
		printf("%d %c %d %c %d %c %d\n",
			pprev*pprev, psign[0], prev*prev, psign[1], top*top, psign[2], atop*atop);
#endif
		return 1;
	}
	return 0;
}

// count the number of ordered pairs of non neg ints <= otop whose squares sum to base
int Find2Square(int base, int prev, int otop, int lim, char *psign)
{
	int ret, cur, sq, atop, rem, top;
	top = lim;
	if(base == 0) {
#ifdef DEBUG
		printf("%d %c %d + 0 + 0\n", prev*prev, psign[0], top*top);
#endif
		return 1;
	}
	psign[1] = '+';
	if(base > 2*top*top) { // one of vals would have to be greater than previous (otop)
		return 0;
	}
	atop = (int)(sqrt(((double)base)/2.0)); // (sloppy) min candidate may be too small
	ret = 0;	// counter of pairs
	for(cur = top; cur >= atop ; cur--) { // for each candidate
		sq = cur*cur;	//subtract square from base
		rem = base - sq;
		if(rem >= 0) {
			if(rem <= sq) { // if rem > sq, next would be bigger than cur (if any)
				psign[2] = '+';
				ret += Find1Square(rem, prev, otop, cur, cur, psign); // see if rem is a perfect square
			}
		} else {
			if(rem <= sq) { // if rem > sq, next would be bigger than cur (if any)
				psign[2] = '-';
				ret += Find1Square(-rem, prev, otop, cur, cur-1, psign); // see if rem is a perfect square
			}
		}
	}
	return ret;
}

// first 2 over shot we ned to subtract base to get to desired value
// count the number of ordered pairs of non neg ints <= otop whose squares sum to base
int Find2SquareNeg(int base, int prev, int otop, int lim, char *psign)
{
	int ret, cur, sq, atop, rem, top;
	top = lim;
	if(base == 0) {
#ifdef DEBUG
		printf("%d %c %d + 0 + 0\n", prev*prev, psign[0], otop*otop);
#endif
		return 1;
	}
	if(base > 2*top*top) { // one of vals would have to be greater than previous (otop)
		return 0;
	}
	psign[1] = '-';
	atop = (int)(sqrt(((double)base)/2.0)); // (sloppy) min candidate may be too small
	ret = 0;	// counter of pairs
	for(cur = top; cur >= atop ; cur--) { // for each candidate
		sq = cur*cur;	//subtract square from base
		rem = base - sq;
		if(rem >= 0) {
			if(rem <= sq) { // if rem > sq, next would be bigger than cur (if any)
				psign[2] = '-';
				ret += Find1Square(rem, prev, otop, cur, cur, psign); // see if rem is a perfect square
			}
		} else {
			if(rem <= sq) { // if rem > sq, next would be bigger than cur (if any)
				psign[2] = '+';
				ret += Find1Square(-rem, prev, otop, cur, cur-1, psign); // see if rem is a perfect square
			}
		}
	}
	return ret;
}

// count the number of ordered triples of non neg ints <= otop whose squares sum to base
int Find3Square(int base, int otop, char *psign)
{
	int ret, cur, sq, atop, rem, top;
	top = otop;
	if(base > 3*top*top) { //top is mac candiate if using it 3 times wuill not work, no hope
		return 0;
	}
	atop = (int)(sqrt(((double)base)/3.0)); // sloppy min candidate if used 3 times
	ret = 0;
	for(cur = top; cur >= atop ; cur--) { // for each candidate subtract square form base 
		sq = cur*cur;					 // and number of ways remainder is a 
		rem = base - sq;				// sum of 2 (no bigger) squares
		psign[0] = '+';
		if(rem >= 0) {
			ret += Find2Square(rem, otop, cur, cur, psign);
		} else {
			ret += Find2SquareNeg(-rem, otop, cur, cur-1, psign);
		}
		if((cur < top) && ( cur > 0)) {
			rem = base + sq;
			psign[0] = '-';
			ret += Find2Square(rem, otop, cur, cur -1, psign);
		}
	}
	return ret;
}

// count the number of ordered quadruples of non neg ints whose squares sum to baseval
int FindFourCount(int baseval)
{
	int top, cur, sq, rem, ret;
	char sign[3];
	ret = 0;
	top = (int)(sqrt((double)baseval));	// max candidate for largest is sqrt(baseval)
	for(cur = top; cur >= (top/2) ; cur--) { // min candidate is 1/2 that
		sq = cur*cur;			// for each candidate, subytract square from baseval
		rem = baseval - sq;		// and count how many triples (<= candidate)
		ret += Find3Square(rem, cur, &(sign[0])); // have squares sum to remainder
	}
	return ret;
}

char inbuf[256];
int main()
{
	int ret, inval;
	if(fgets(&(inbuf[0]), 255, stdin) == NULL) {
		fprintf(stderr, "read failed on input\n");
		return -1;
	}
	if(sscanf(&(inbuf[0]), "%d", &inval) != 1) {
		fprintf(stderr, "scan failed on input\n");
		return -1;
	}
	ret = FindFourCount(inval);
	printf("%d\n", ret);
	return 0;
}

