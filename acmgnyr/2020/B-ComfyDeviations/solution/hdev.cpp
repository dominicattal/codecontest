/*
 * Comfy Deviations
 * Author: John Buck
 * ICPC GNYR 2020 Regional Contest
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define	NUM_TEMPS	10
#define	COMFY_ZONE	1.0

double CalcSD(double t[])
{
	int i;
	double  mean = 0, sum = 0, tmp;

	for(i = 0; i < NUM_TEMPS; i++){
		mean += t[i];
	}
	mean /= (double)NUM_TEMPS;
	for(i = 0; i < NUM_TEMPS; i++){
		tmp = t[i] - mean;
		sum += tmp*tmp;
	}
	sum /= (double)(NUM_TEMPS-1);
	return((double)::sqrt(sum));
}

int main(int argc, char **argv)
{
	double temps[NUM_TEMPS], sd;
	int i;

	for(i = 0; i < NUM_TEMPS; i++){
		scanf("%lf", &(temps[i]));
	}
	sd = CalcSD(&(temps[0]));
	printf("%sCOMFY\n", sd <= COMFY_ZONE ? "" : "NOT ");
#ifdef DEBUG
	printf("%f\n", sd);
#endif
	return(0);
}

