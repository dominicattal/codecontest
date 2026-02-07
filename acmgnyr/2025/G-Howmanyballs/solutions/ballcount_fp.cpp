/*
 * Fred Pickel, June 2025
 * How many balls?
 * ICPC East Division Contest
 * Greater NY Region
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

typedef long long LL;
//#define DEBUG
//#define DEBUG1

#define MAXX	1000000
int p, q, q2p, D;

LL GCD(LL a, LL b) {
	LL c;
	if(a == 0) return b;
	if(b == 0) return a;
	if(a < 0) a = -a;
	if(b < 0) b = -b;
	c = a % b;
	while (c > 0) {
		a = b; b = c;
		 c= a%b;
	}
	return b;
}

LL isSquare(LL d)
{
	LL r;
	double dr;
	if(d < 0) {
		return -1;
	}
	dr = sqrt((double)d);
	r = LL(dr);
	while(r*r < d) {
		r++;
	}
	if((r*r) == d) return r;
	return -1;
}

LL BF(LL lim, LL *py)
{
	LL x, y, qp2, den, ds;
	LL mB, C, d;
	den = ( 2*p);
	qp2 = 2*(q-p);
	for(x = 1; x <= lim ; x++) {
		mB = qp2*x + p;
		C = p*x*(x -1);
		d = mB*mB - 4*p*C;
		if((ds = isSquare(d)) >= 0){
			if((mB > ds) && (((mB-ds)%den) == 0)) {
				y = (mB - ds)/den;
				if(y < x) {
					*py = x;
					return y;
				} else {
					*py = y;
					return x;
				}
			} else if(((mB+ds)%den) == 0){
				y = (mB + ds)/den;
				if(y < x) {
					*py = x;
					return y;
				} else {
					*py = y;
					return x;
				}
			}
		}
	}
	return -1;
}

int validate( LL x, LL y)
{
	LL g, pt, qt, p1, q1, xl, yl, pl, ql;
	xl = x; yl = y;
	pl = p; ql = q;
	pt = 2*x*y;
	qt = (x + y)*(x + y -1);
	g = GCD(pt, qt);
	if(g <= 0) {
		return -2;
	}
	p1 = pt/g;
	q1 = qt/g;
	if((p1 == pl) && (q1 == ql)) {
		return 0;
	}
	printf("p %lld q %lld\n", p1,q1);
	return -2;
}


	
char inbuf[1024];

int main()
{
	LL sqd, x, y, v;
#ifdef DEBUG
	while(1) {
#endif
	if(fgets(&(inbuf[0]), sizeof(inbuf), stdin) == NULL) {
		fprintf(stderr, "read failed on nvals\n");
		return -1;
	}
	if((sscanf(&(inbuf[0]), "%d %d", &p, &q)) != 2) {
		fprintf(stderr, "scan failed on counts\n");
		return -2;
	}
	if((p < 1) || (q < (2*p -1))) {
		fprintf(stderr, "p %d < 1 OR q %d < 2*p - 1\n", p, q);
		return -3;
	}
	if((p == 1) && (q == 1)){
		printf("1 1\n");
	} else if(q == (2*p - 1)) {
		x = BF(p-1, &y);
		printf("%lld %lld\n", x, y);
	} else if (q == 2*p) {
		printf("1 3\n");
	} else {
		q2p = q - 2*p;
		D = q*q2p;
		x = BF(MAXX, &y);
		if(x < 0) {
			printf("impossible\n");
		} else {
			printf("%lld %lld\n", x, y);
		}
	}
#ifdef DEBUG
	}
#endif
	return 0;
}

