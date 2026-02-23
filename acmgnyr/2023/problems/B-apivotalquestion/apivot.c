/*
 * Correct(?) solution to A Pivotal Question
 *
 * John Buck, Greater NY Chief Judge
 * Sept 8, 2023
 */
#include <stdio.h>

#define	MAXNUM	1000000
int valsp[MAXNUM];
unsigned char cand[MAXNUM];

int main(int argc, char **argv)
{
	int i, n, nPiv = 0;
	int min = 0;
	int max = 0;
	scanf("%d", &(n));
	for(i = 0; i < n; i++){
		scanf("%d", &(valsp[i]));
		if(valsp[i] >= min){
			// this is a candidate
			cand[i] = 1;
			// remember biggest
			min = valsp[i];
		}
	}
	// now the right side - prime max value
	if(n > 1){
		for(i = n; --i >= 0; ){
			if(valsp[i] < max || max == 0){
				cand[i] &= 1;
				if(cand[i] == 1){
					nPiv++;
				}
				max = valsp[i];
			} else {
				cand[i] = 0;
			}
		}
	} else {
		// one value, one pivot point
		nPiv = 1;
	}
	printf("%d", nPiv);
	if(nPiv > 100){
		nPiv = 100;
	}
	for(i = 0; i < n && nPiv > 0; i++){
		if(cand[i] == 1){
			printf(" %d", valsp[i]);
			nPiv--;
		}
	}
	printf("\n");
	return(0);
}

