/*
 * A - Cornhole solution
 * 2022 GNY Regional
 * Author: John Buck
 */
#include <stdio.h>

int main(int argc, char **argv)
{
	int h1, b1, h2, b2;

	scanf("%d %d %d %d", &(h1), &(b1), &(h2), &(b2));
	h1 = h1*3 + b1;
	h2 = h2*3 + b2;
	if(h1 > h2){
		printf("1 %d\n", h1 - h2);
	} else if(h2 > h1){
		printf("2 %d\n", h2 - h1);
	} else {
		printf("NO SCORE\n");
	}
	return(0);
}
