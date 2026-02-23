/*
 * Plus Minus Four Squares solution
 * John Buck - Chief Judge GNY
 *
 * Mickey Mouse recursive solution
 */
#include <stdio.h>
#include <stdlib.h>

#define MAXNUM  5000

int squares[MAXNUM];

int docount(int total, int prevmul, int mul, int idx, int depth)
{
	int v, cnt = 0, i, diff;
	int is;

	if (++depth == 5) {
		return(0);
	}
	if (mul != prevmul) {
		idx--;
	}
	for (i = idx; i >= 1; i--) {
		v = squares[i];
		diff = total - mul * v;
		if (diff == 0) {
			cnt++;
		}
		cnt += docount(diff, mul, 1, i, depth);
		cnt += docount(diff, mul, -1, i, depth);
	}
	return(cnt);
}

int main()
{
	int ns, sum, i, n;

	/* A little bit of DP */
	for(ns = 0, n = 0; n <= MAXNUM; ns++){
		n = ns * ns;
		squares[ns] = n;
	}
	if (scanf("%d", &(sum)) != 1) {
		fprintf(stderr, "Bad input\n");
		return(1);
	}
	/* Well... it complicates the algorithm, so handle 0 special */
	if(sum == 0){
		printf("1\n");
	} else {
		/* Find biggest square <= sum */
		for (i = ns - 1; i >= 0; i--) {
			if (squares[i] <= sum) {
				break;
			}
		}
	
		printf("%d\n", docount(sum, 1, 1, i, 0));
	}
	return(0);
}

