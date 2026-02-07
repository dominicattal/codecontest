#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>

#undef TEST

unsigned long GetTest(unsigned long n)
{
	unsigned long ret;
	if((n & 1) == 0) {
		ret = n/2;
		ret = ret*(n+1) + 1;
	} else {
		ret = (n+1)/2;
		ret = ret*n + 1;
	}
	return ret;
}

/*
	the block begining with n has n elts so the last entry in the block starting with n is
	 at index n*(n+1)/2 and the first index is at 1 + n*(n-1)/2
	 so we solve (multiply by 2) 2 + n^2 + n  = 2*index
	 n = (-1 + sqrt(1 + 8*(index - 1))/2
*/
int GetEntry(unsigned long index, int *num, int *denom)
{
	double dindex, dn;
	unsigned long n, test;
	dindex = (double)index;
	dn = 0.5*(-1.0 + sqrt(1 + 8*(dindex - 1.0)));
	n = (unsigned long)dn;
	test = GetTest(n);
	while (test > index) {
		n = n-1;
		test = GetTest(n);
	}
	*denom = n+1;
	*num = index - test;

	return 0;

}

int GCD(int a, int b)
{
	unsigned long c;
	if(a == 0) return b;
	if(b == 0) return a;
	if(a < 0) a = -a;
	if(b < 0) b = -b;
	c = a % b;
	while(c > 0) {
		a = b; b = c;
		c = a % b;
	}
	return b;
}

char inbuf[256];
int main()
{
	unsigned long index;
	int num, denom, ret, g;
#ifdef TEST
	while(1) {
#endif
	if(fgets(&(inbuf[0]), 255, stdin) == NULL) {
		fprintf(stderr, "read failed on dimensions\n");
		return -2;
	}
	if(sscanf(&(inbuf[0]), "%lu", &index) != 1) {
		fprintf(stderr, "scan failed on input index\n");
		return -3;
	}
	if(index == 0) {
		fprintf(stderr, "zero index invalid\n");
		return -4;
	}
	if((ret = GetEntry(index, &num, &denom)) != 0) {
		fprintf(stderr, "Get error %d\n", ret);
		return ret;
	}
	if(num == 0) {
		printf("%d\n", denom);
	} else {
		g = GCD(num, denom);
		printf("%d %d/%d\n", denom, num/g, denom/g);
	}
#ifdef TEST
	}
#endif
	return 0;
}




