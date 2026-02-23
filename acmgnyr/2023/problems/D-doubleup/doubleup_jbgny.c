/*
 * Double Up Solution
 * John Buck, Greater New York Chief Judge
 * Sept 10, 2023
 * Has some utility routines to convert big powers of 2 to strings and vice versa
 * Uses int128 types.
 * Define DEBUG for cryptic debugging info.
 */
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <memory.h>

typedef	unsigned __int128 BIGUINT;

BIGUINT pow2tab[128];

BIGUINT atoq(const char *szNum)
{
	BIGUINT uResult = 0;

	while(isdigit(*szNum)){
		uResult *= 10;
		uResult += (*szNum - '0');
		szNum++;
	}
	return(uResult);
}

const char *qtoa(BIGUINT n)
{
	static char szNumBuf[128];
	char *s = &(szNumBuf[sizeof(szNumBuf)-1]);

	*s-- = '\0';
	do {
		*--s = (n%10) + '0';
		n /= 10;
	} while(n != 0);
	return(s);
}

int
Trim(int *p, int n)
{
	int i, ismall = 0, nsmall = *p;

	for(i = 1; i < n; i++){
		if(p[i] <= nsmall){
			nsmall = p[i];
			ismall = i;
		}
	}
#ifdef DEBUG
	printf("Removing index %d (%d) n=%d\n", ismall, nsmall, n);
#endif
	memmove(p+ismall, p+ismall+1, (n-ismall)*(sizeof(int)));
	n--;
#ifdef DEBUG
	for(i = 0; i < n; i++){
		printf("%d: %d\n", i, p[i]);
	}
#endif
	return(n);
}

int Merge(int *p, int n)
{
	int i;

	for(i = 1, p++; i < n; ){
#ifdef DEBUG
		printf("i=%d  p[-1]=%d *p=%d\n", i, p[-1], *p);
#endif
		if(p[-1] == *p){
			p[-1]++;
#ifdef DEBUG
			printf("Merge: i=%d n=%d p[-1]=%d\n", i, n, p[-1]);
#endif
			n--;
			memmove(p, p+1, (n-i)*(sizeof(int)));
		} else {
			i++;
			p++;
		}
	}
#ifdef DEBUG
	printf("returning %d\n", n);
#endif
	return(n);
}

int main(int argc, char **argv)
{
	BIGUINT bit;
	int *pInVals;
	int idx, cnt, i;
	char szBuf[128];

	// build log2 table, about 100x faster than doing log2() all the time for up to 1000 nums
	for(bit = 1, idx = 0; idx < 128; idx++){
		pow2tab[idx] = bit;
		bit <<= 1;
	}
	// # values
	scanf("%d", &(cnt));
	pInVals = (int *)malloc(cnt * sizeof(int));
	for(idx = 0; idx < cnt; idx++){
		scanf("%s", &(szBuf[0]));
		// get log2 of input
		bit = atoq(&(szBuf[0]));
		for(i = 0; i < 128; i++){
			if(pow2tab[i] == bit){
				break;
			}
		}
		if(i >= 128){
			printf("Bad input data %s\n", &(szBuf[0]));
			return(1);
		}
		pInVals[idx] = i;
	}

	// Merge and remove
	for(;cnt > 1;){
		i = Merge(pInVals, cnt);
		if(i != cnt){
			cnt = i;
			continue;
		}
		/* Remove smallest */
		cnt = Trim(pInVals, cnt);
	}
	printf("%s\n", qtoa(((BIGUINT)1) << (*pInVals)));
	free(pInVals);
	
	return(0);
}
