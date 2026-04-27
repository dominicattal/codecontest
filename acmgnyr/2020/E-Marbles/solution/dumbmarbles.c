/*
 * Marples
 * Author: Fred Pickel
 * ICPC GNYR 2020 Regional Contest
 *
 * Dumb brute force method to solve marble problem
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

//#define TEST1

#define MAX_N	1000
typedef unsigned char BYTE;
typedef unsigned long DWORD;
typedef int LL;
int p, q, N, M, r, g, pm, qm, pn, qn, rm, gm, rn, gn;

int GCD(int a, int b)
{
	int c;
	if(a < 0) a = -a;
	if(b < 0) b = -b;
	if(a == 0) return b;
	if(b == 0) return a;
	c = a % b;
	while(c > 0) {
		a = b;
		b = c;
		c = a % b;
	}

	return b;
}

int p1, q1;
// find p1, q1 so Prob(r,g) = p1/q1
void FindProb(int r, int g, int *pp, int *pq)
{
	int t;
	p1 = 2*r*g;
	q1 = (r+g)*(r+g - 1);
	t = GCD(p1, q1);
	if(t > 1) {
		p1 /= t;
		q1 /= t;
	}
	*pp = p1;
	*pq = q1;
}

void CheckSetRS(int rm, int gm)
{
	if(rm > gm) {
		r = gm; g = rm;
	} else {
		r = rm; g = gm;
	}
}

// find rm + gm = N, rn + gn = N with
// Prob (rm,gm) >= p/q >= Prob(rn,gn)
int Init()
{
	int diff;
	if(p == 0) {
		r = 0; g = N;
		return 1;
	}
	if(p == q) {
		if((N > 2) || (M < 2)) {
			return -1;
		}
		r = g = 1;
		return 1;
	}
	rm = (p*N)/q;
	if(rm < 1) rm = 1;
	gm = N - rm;
	FindProb(rm, gm, &pm, &qm);
	diff = (p*qm) - (q*pm) ;
	if(diff == 0) {
		CheckSetRS(rm, gm);
		return 1;
	}
	if(diff < 0) {	//Prob(rm,gm) > p/q scan to find
		rn = rm - 1; gn = gm+1;
		FindProb(rn, gn, &pn, &qn);
		diff = (p*qn) - (q*pn) ;
		while ((diff < 0) && (rm > 1)){
			rm = rn; gm = gn; pm = pn; qm = qn;
			rn = rm - 1; gn = gm+1;
			FindProb(rn, gn, &pn, &qn);
			diff = (p*qn) - (q*pn) ;
		}
		if(diff == 0) {
			CheckSetRS(rn, gn);
			return 1;
		}
	} else {	//Prob(rm,gm) <> p/q
		rn = rm; gn = gm; pn = pm; qn = qm;
		rm = rn+1; gm = gn -1;
		FindProb(rm, gm, &pm, &qm);
		diff = (p*qm) - (q*pm) ;
		while((rm < gm) && (diff > 0)) {
			rn = rm; gn = gm; pn = pm; qn = qm;
			rm = rn+1; gm = gn -1;
			FindProb(rm, gm, &pm, &qm);
			diff = (p*qm) - (q*pm) ;
		}
		if(diff == 0) {
			CheckSetRS(rm, gm);
			return 1;
		}
		if(diff > 0) {
			return -1;
		}
	}
	return 0;
}

// go thru all possible sums from N to M
// maintaining Prob (rm,gm) >= p/q >= Prob(rn,gn)
// until one is equal or we run out of space
int FindRS()
{
	int diffm, diffn, ret;
	ret = Init();
	if(ret == 1) return 0;
	else if(ret < 0) return -2;
	while((rm+gm) < M) {
		gm++; rn++;
		FindProb(rm, gm, &pm, &qm);
		diffm = (p*qm) - (q*pm) ;
		if(diffm == 0) {
			CheckSetRS(rm, gm);
			return 0;
		}
		FindProb(rn, gn, &pn, &qn);
		diffn = (p*qn) - (q*pn) ;
		if(diffn == 0) {
			CheckSetRS(rn, gn);
			return 0;
		}
		if(diffm < 0) {	//Prob(rm,gm) > p/q 
			rn = rm-1; gn = gm + 1;
			FindProb(rn, gn, &pn, &qn);
			diffn = (p*qn) - (q*pn) ;
			if(diffn == 0) {
				CheckSetRS(rn, gn);
				return 0;
			}
#ifdef EXTRA
			if(diffn < 0) { // should not have to do this
				rm = rn; gm = gn;
				rn = rm-1; gn = gm + 1;
				FindProb(rn, gn, &pn, &qn);
				diffn = (p*qn) - (q*pn) ;
				if(diffn == 0) {
					CheckSetRS(rn, gn);
					return 0;
				}
			}
#endif
		} else {	//Prob(rm,gm) < p/q 
			rn = rm; gm = gn;
			rm++; gm--;
			FindProb(rm, gm, &pm, &qm);
			diffm = (p*qm) - (q*pm) ;
			if(diffm == 0) {
				CheckSetRS(rm, gm);
				return 0;
			}
#ifdef EXTRA
			if(diffm > 0) { // should not have to do this
				rn = rm; gm = gn;
				rm++; gm--;
				FindProb(rm, gm, &pm, &qm);
				diffm = (p*qm) - (q*pm) ;
				if(diffm == 0) {
					CheckSetRS(rm, gm);
					return 0;
				}
			}
#endif
		}
	}
	return -1;
}

char inbuf[256];

int main()
{
	int ret;
#ifdef TEST1
	while(1) {
#endif
	if(fgets(&(inbuf[0]), 255, stdin) == NULL) {
		fprintf(stderr, "read failed on input data\n");
		return -7;
	}
	if(sscanf(&(inbuf[0]), "%d %d %d %d",
		&p, &q, &N, &M) != 4) {
		fprintf(stderr, "scan failed on input data\n");
		return -8;
	}
#ifdef TEST1
	if(p == 0) {
		return 0;
	}
#endif
	if((p < 0) || (q < p) || (N > MAX_N)) {
		fprintf(stderr, "Invalid input %d %d %d %d\n", p, q, N, M);
		return -9;
	}
	ret = FindRS();
	if(ret == 0) {
		printf("%d %d\n", r, g);
	} else {
		printf("NO SOLUTION\n");
	}
	fflush(stdout);
#ifdef TEST1
	}
#endif
	return 0;
}
