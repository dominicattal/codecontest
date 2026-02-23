/*
 * Cornhusker solutions
 *
 * John Buck - Greater New York Regional Chief Judge
 * Sept 8, 2023
 */
#include <stdio.h>

int main(int argc, char **argv)
{
	int nEar, nKWF;
	int na, nl, nTot;
	int i;

	nTot = 0;
	for(i = 0; i < 5; i++){
		scanf("%d %d", &(na), &(nl));
		nTot += (na * nl);
	}
	nTot /= 5;
	scanf("%d %d", &(nEar), &(nKWF));
	printf("%d\n", (nTot * nEar)/nKWF);
	return(0);
}
