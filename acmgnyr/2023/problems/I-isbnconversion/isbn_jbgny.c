/*
 * Solution to ISBN Conversion
 * John Buck, Greater New York Chief Judge
 * Sept 12, 2023
 *
 * Extremely ugly, stream of consciousness coding.
 * These isn't even "greedy" - it's something else.
 * Lots of groaning while reading this code.
 */
#include <stdio.h>
#include <ctype.h>
#include <string.h>

int IsValidISBN(const char *s)
{
	int nHyphen = 0;
	char cPrev = '\0';
	int nFirst = 1;
	int sum = 0;
	int nsum = 9 + 7*3 + 8;
	int nDigits = 10;
	int nToggle = 1;

	while(*s != '\0' && nDigits > 1 && nHyphen < 3){
		if(*s == '-'){
			if(nFirst == 1 || cPrev == '-'){
				return(-1);
			}
			nHyphen++;
		} else if(!isdigit(*s)){
			return(-1);
		} else {
			sum += nDigits*(*s - '0');
			nsum += (*s - '0')*(nToggle*2 + 1);
			nToggle ^= 1;
			nDigits--;
		}
		nFirst = 0;
		cPrev = *s;
		s++;
	}
	if(*s == '-'){
		if(nHyphen == 3){
			return(-1);
		}
		s++;
	}
	if(nDigits != 1 || (!isdigit(*s) && *s != 'X')){
		return(-1);
	}
	if(*s == 'X'){
		sum += 10;
	} else {
		sum += (*s - '0');
	}
	s++;
	if(*s != '\0' || (sum%11) != 0){
		return(-1);
	}
	return(nsum);
}

int main(int argc, char **argv)
{
	int t, nsum, nLen;
	char szBuf[128];
	scanf("%d", &(t));
	while(t-- > 0){
		scanf("%s", &(szBuf[0]));
		if((nsum = IsValidISBN(&(szBuf[0]))) == -1){
			printf("invalid\n");
		} else {
			nLen = strlen(&(szBuf[0]));
			snprintf(&(szBuf[nLen-1]), sizeof(szBuf) - nLen - 1, "%d", (10 - (nsum%10))%10);
			printf("978-%s\n", &(szBuf[0]));
		}
	}
	return(0);
}
