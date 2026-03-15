/*
 * Greater NY Regional 2022
 * pythagorean Triples
 * Fred Pickel
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

//#define DEBUG	// print all triples found, labelling duplicates
//#define DEBUG1	// print a message if no element of a triple is the input value

#ifndef WIN32
typedef unsigned long long DDWORD;
#else
typedef unsigned __int64 DDWORD;
#endif
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;

int hyp_prim_cnt, hyp_not_cnt, side_prim_cnt, side_not_cnt; // output values

// to detect duplicate triples we keep a hash table of previously found triples
// this is a hash entry with a < b < c to keep uniqueness
typedef struct _found_trip_ {
	struct _found_trip_ *pnxt;
	int a, b, c;	// a < b < c
} FOUND_TRIP, *PFOUND_TRIP;

FOUND_TRIP found[256];	// hash entry cache
int nxt_found = 0;	// where we are in cache

PFOUND_TRIP triphash[257];	// hash table

int GCD(int a, int b)
{
	int c;
	if(a < 0) a = -a;
	if(b < 0) b = -b;
	if(a == 0) return b;
	if(b == 0) return a;
	while(b != 0) {
		c = a % b;
		a = b;
		b = c;
	}
	return a;
}

int GCD3(int a, int b, int c)
{
	int d;
	d = GCD(a, b);
	return GCD(c, d);
}

// make a triple from inputs m and n swap a and b to get a < b < c
// multiply by fact to get a non-primitive triple
int makepf(int a, int b, int c, PFOUND_TRIP pf)
{
	int t;
	if(a > b) {
		t = a; a = b; b = t;
	}
	pf->pnxt = NULL;
	pf->a = a;
	pf->b = b;
	pf ->c = c;

	return 0;
}

// verify that a^2 + b^2 == c^2 and that the input (n) is one of a, b or c
int validate(int n, PFOUND_TRIP pf)
{
	int rhs, lhs;
	rhs = pf->c*pf->c;
	lhs = pf->a*pf->a;
	lhs += pf->b*pf->b;
	if(lhs != rhs) {
		return -1;
	}
	if((n != pf->a) && (n != pf->b) && (n != pf->c)){
#ifdef DEBUG1	// serious bug
		printf("no match %d %d %d\n", pf->a, pf->b, pf->c);
#endif
		return -2;
	}
	return 0;
}

// validate triple in pf, then see if it is in hash table
// if in return 0 (no triple added) else insert in hash table and return 1
int checkHash(int n, PFOUND_TRIP pf)
{
	int hash;
	PFOUND_TRIP pscan;
	if(validate(n, pf) != 0) {
#ifdef DEBUG
		printf("INAL: %d %d %d\n", pf->a, pf->b, pf->c);
#endif
		return 0;
	}
	hash = pf->a % 257;;
	hash = (5*hash + pf->b) % 257;
	hash = (5*hash + pf->c) % 257;
	pscan = triphash[hash];
	if(pscan == NULL) {
		triphash[hash] = pf;
		return 1;
	}
	while(1) {
		if((pscan->a == pf->a) && (pscan->b == pf->b) && (pscan->c == pf->c)) {
#ifdef DEBUG
			printf("DUP: %d %d %d\n", pf->a, pf->b, pf->c);
#endif
			return 0;
		}
		if(pscan->pnxt == NULL) {
			break;
		}
		pscan = pscan->pnxt;
	}
	// not there add it and return 1;
	pscan->pnxt = pf;
	return 1;
}

int CountSumSquares(int n)
{
	int a, b, sqa, sqb, lim, sqn;
	PFOUND_TRIP pf;
	sqn = n*n;
	lim = (int)(sqrt(0.5*((double)sqn)));
	b = n-1;
	sqb = b*b;
	a = 1; sqa = 1;
	while(b >= lim) {
		while((sqa + sqb) < sqn) {
			a++;
			sqa = a*a;
		}
		if((sqa + sqb) == sqn) {
			pf = &(found[nxt_found]);
			if(makepf(a, b, n, pf) == 0) {
				if(checkHash(n, pf) == 1) {
					nxt_found++;
					if(GCD3(a, b, n) == 1) {
						hyp_prim_cnt++;
					} else {
						hyp_not_cnt++;
					}
#ifdef DEBUG
					printf("%d %d %d\n", pf->a, pf->b, pf->c);
#endif
				}
			}
		}
		b--;
		sqb = b*b;
	}
	return 0;
}

int CountDiffSquares(int n)
{
	DDWORD c, b, sqc, sqb, sqn;
	PFOUND_TRIP pf;
	sqn = n*n;
	c = (sqn+1)/2; sqc = c*c;
	b = c - 1;	sqb = b*b;
	while (c > n) {
		while((sqc - sqb) < sqn) {
			b--;
			sqb = b*b;
		}
		if((sqc - sqb) == sqn) {
			pf = &(found[nxt_found]);
			if(makepf(n, (int)b, (int)c, pf) == 0) {
				if(checkHash(n, pf) == 1) {
					nxt_found++;
					if(GCD3((int)c, (int)b, n) == 1) {
						side_prim_cnt++;
					} else {
						side_not_cnt++;
					}
#ifdef DEBUG
					printf("%d %d %d\n", pf->a, pf->b, pf->c);
#endif
				}
			}
		}
		c--;
		sqc = c*c;
		if(b >= c) {
			b = c-1;
			sqb = b*b;
		}
	}
	return 0;
}

int GetTripleCount(int n)
{
	hyp_prim_cnt = hyp_not_cnt = side_prim_cnt = side_not_cnt = 0;
	CountSumSquares(n);
	CountDiffSquares(n);
	return 0;
}

char inbuf[256];
int main()
{
	int ret, inval;
	if(fgets(&(inbuf[0]), 255, stdin) == NULL) {
		fprintf(stderr, "read failed on input\n");
		return -1;
	}
	if(sscanf(&(inbuf[0]), "%d", &inval) != 1) {
		fprintf(stderr, "scan failed on input\n");
		return -2;
	}
	ret = GetTripleCount(inval);
	if(ret != 0) {
		fprintf(stderr, "Count error %d\n", ret);
		return -3;
	}
	printf("%d %d %d %d\n", hyp_prim_cnt, hyp_not_cnt, side_prim_cnt, side_not_cnt);
	return 0;
}

