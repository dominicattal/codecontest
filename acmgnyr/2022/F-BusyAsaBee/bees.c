/*
 * Bee's honeycomb builder
 * Greater NY Regional 2022 Solution
 * John Buck
 *
 * Longer than it has to be, but commented for readability.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	long long m, n, cnt, oddrows, evenrows, nsub;

	if(scanf("%lld %lld", &(m), &(n)) != 2){
		fprintf(stderr, "Bad input\n");
		return(1);
	}

	oddrows = (n+1)/2;
	evenrows = n - oddrows;

	//(2 top pointy edges)*wid*(2 bot pointy edges)
	cnt = (2 * m * 2) * (oddrows);
	if((n & 1) == 0){
		cnt += (m-1)*2;	// last even row bottoms
	}

	// Verticals now, even rows first
	cnt += evenrows*m;	// vertical edges of even rows
	// add in verticals on odd rows
	cnt += oddrows * (m+1);
	// need to subtract out edges that would complete hex
	nsub = oddrows * m;
	nsub += evenrows * (m-1);
	// subtract them out, but only half of them (hex's share a side)
	// pull off an extra edge if we are pulling off an odd number
	cnt -= (nsub/2 + (nsub & 1));
	// add 1 since the last wall will complete at least one cell, but still
	// takes an hour to build.
	printf("%lld\n", cnt+1);
	return(0);
}
