/*
 * Stern's Sequence
 * Author: Fred Pickel
 * ICPC GNYR 2020 Regional Contest
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef unsigned int DWORD;

//#define TEST

#ifdef WIN32
typedef unsigned __int64 LLONG;
#else
typedef unsigned long long LLONG;
#endif

#define MODULUS	998244353
#define MAX_N	400000

int GCD(int p, int q) {
	int c = p % q;
	while(c != 0) {
		p = q;
		q = c;
		c = p % q;
	}
	return q;
}

/* basic soln:
 *	suppose k = K(p,q) is the value for which p = b(k) and q = b(k+1)
 * if p = q = 1 then k = 1
 * otherwise
 *	if(p > q) then p = S(n,2*m+1) and q = S(n,2*m+2) = S(n-1,m+1) for some m and large enough n
 *			p = S(n,2*m+1) = S(n-1,m) + S(n-1,m+1) so S(n-1,m) = p-q
 *		so k = K(p,q) = 2*m+1 = 2*K(p-q,q) + 1
 * if(p < q) then q = S(n,2*m+1) and p = S(n,2*m) = S(n-1,m) for some m and large enough n
 *			q = S(n,2*m+1) = S(n-1,m) + S(n-1,m+1) so S(n-1,m+1= q-p
 *		so k = K(p,q) = 2*m = 2*K(p,q-p)
 *
 * issue: for K(1,bignumber) we would need to recurse bignumber -1 times (stack overflow?)
 *
 *	if(p < q) and q/p = r then K(p,q) 2*K(p,q-p) = 2*2*K(p,q-2*p) = ... (2^r)*K(p,q-r*p)
 *
 *  if( p > q) and p/q = r then K(p,q) = 2*K(p-q,q) + 1 =
 *			2*(2*K(p-2*q,q) + 1) + 1 = 2*2*K(p-2*q,q) + 2 + 1 =
 *			2*(2*(2*K(p-3*q,q) + 1) + 1) + 1 = 2*2*2*K(p-3*q,q) + 2*2 + 2 + 1 =
 *			2^r*K(p-r*q,q) + SUM (i = 1 to r)(2^(i-1)) = 2^r*K(p-r*q,q) + 2^r -1
 *					
 */

DWORD K_basic(int p, int q)
{
	if((p == 1) && (q == 1)) {
		return 1;
	}
	if(p > q) {
		return ((2*K_basic(p-q,q) + 1) % MODULUS);
	} else {
		return ((2*K_basic(p,q-p)) % MODULUS);
	}
}

LLONG Pow2mod(int n)
{
	int mask;
	LLONG pow, res;
	pow = 2;
	res = 1;
	mask = 1;
	while((mask != 0) && (mask <= n)) {
		if(mask & n) {
			res = (res * pow) % MODULUS;
		}
		pow = (pow*pow) % MODULUS;
		mask <<= 1;
	}
	return res;
}

DWORD K(int p, int q)
{
	LLONG ktmp, pow;
	int r;
	if((p == 1) && (q == 1)) {
		return 1;
	}
	if(p > q) {
		r = p/q;
		if(q == 1) r--;
		if(r > 3) {
			ktmp = K(p-r*q,q) + 1;
			pow = Pow2mod(r);
			ktmp = (ktmp*pow - 1)  % MODULUS;
			return (DWORD)ktmp;
		} else {
			return ((2*K(p-q,q) + 1) % MODULUS);
		}
	} else {
		r = q/p;
		if(p == 1) r--;
		if(r > 3) {
			ktmp = K(p,q - r*p);
			pow = Pow2mod(r);
			ktmp = (ktmp*pow)  % MODULUS;
			return (DWORD)ktmp;
		} else {
			return ((2*K(p,q-p)) % MODULUS);
		}
	}
}
char inbuf[256];

int main()
{
	DWORD p, q, cnt;
	int g;
#ifdef TEST
	DWORD cntb;
	while(1) {
#endif
	if(fgets(&(inbuf[0]), 255, stdin) == NULL) {
		fprintf(stderr, "read failed on input data\n");
		return -3;
	}
	if(sscanf(&(inbuf[0]), "%u %u", &p, &q) != 2){
		fprintf(stderr, "scan failed on problem data\n");
		return -3;
	}
	if((p < 1) || (p > MAX_N) || (q < 1) || (q > MAX_N)) {
		fprintf(stderr, "p %u or q %u not in range 1 ... %d\n", p, q, MAX_N);
		return -5;
	}
	if((g = GCD(p,q)) != 1) {
		fprintf(stderr, "GCD of p %u and q %u is %d not 1\n", p, q, g);
#ifndef TEST
		return -5;
#else
		continue;
#endif
	}
	cnt = K(p,q);
#ifdef TEST
	if((p <= 4000) && (q <= 4000)) {
		cntb = K_basic(p,q);
		printf("%u %u\n", cnt, cntb);
	} else {
		printf("%u\n", cnt);
	}
	}
#else
	printf("%u\n", cnt);
#endif
	return 0;
}
