/*
 * Bottle Return
 * Author: John Buck
 * ICPC GNYR 2020 Regional Contest
 */
#include <stdio.h>

#define	NUM_BOTYPES	6
#define	DEP_CENTS	5

int main(int argc, char **argv)
{
	int nSum, i, v;

	for(i = 0, nSum = 0; i < NUM_BOTYPES; i++, nSum += v){
		if(scanf("%d", &(v)) != 1){
			break;
		}
	}
	printf("%d\n", nSum * DEP_CENTS);
	return 0;
}


