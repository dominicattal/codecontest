/*
 * Evenly Separated Strings
 * Author: Fred Pickel
 * ICPC GNYR 2020 Regional Contest
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

//#define TEST

typedef unsigned char BYTE;
#define NO_CHAR 0xff
#define TWO_CHARS	0xfe
BYTE loc[26];	// index of first instance of a-z in str

/*
 * if 3 of same char in string and even number of chars between 1st and 2nd
 * and even number of chars between 2nd and 3rd then od number between 
 * 1st and 3rd (1-2 psace + 2-3space + 2nd char so 3 of same char fails
 * record location of first instance of char
 * on second instance if offset from first is odd, mark as seen two
 * else return 0
 * if char marked as seen 2  return 0
 * when you see a non-a-z return 1
 */
int ChkEvenSpace()
{
	int i, c, l;
	memset(&(loc[0]), NO_CHAR, 26);
	i = 0;
	while(1) {
		c = getchar();
		if((c < 'a') || (c > 'z')) {
			return 1;
		}
		c -= 'a';
		l = loc[c];
		if(l == NO_CHAR) {
			loc[c] = i;
		} else if(l == TWO_CHARS) {	// 3 of same char
			break;
		} else {
			if(((i - loc[c]) & 1) == 0){	// odd spearation
				break;
			}
			loc[c] = TWO_CHARS;
		}
		i++;
	}
	/* suck up remaining chars */
	while(1) {
		c = getchar();
		if((c < 'a') || (c > 'z')) {
			return 0;
		}
	}
	/* not reached? */
	return 0;
}

int main()
{
	int ret;
#ifdef TEST
	int i;
	for(i = 0; i < 10 ; i++) {
#endif
	ret = ChkEvenSpace();
	if(ret == 0) {
		printf("NO\n");
	} else {
		printf("YES\n");
	}
#ifdef TEST
	}
#endif
	return 0;
}
