#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

typedef long long LL;

//#define DEBUG
//#define DEBUG1

#define MAXX	1000000
LL p, q, q2p, D;

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

int test(LL r, LL g)
{
	LL num, den;
	num = r*g;
	if((r+g)& 1) {
		den = (r+g-1)/2;
		den *= (r+g);
	} else {
		den = (r+g)/2;
		den *= (r+g-1);
	}
	if((p*den) == (q*num)) {
		return 0;
	} else  if((p*den) > (q*num)) {
		return 1;
	}
	return -1;
}

LL BF(LL lim, LL *py)
{
	LL r, g, gmin, gmax, step;
	int ret;
	for(r = 1; r <= lim ; r++) {
		if(r < 10) step = 10;
		else step = r;
		gmin = r+1;
		gmax = gmin + step;
		while((ret = test(r,gmax)) < 0) {
			gmin = gmax;
			step *= 2;
			gmax += step;
		}
		if(ret == 0) {
			*py = gmax;
			return r;
		}
		while(gmin < (gmax - 1)) {
			step /= 2;
			g = (gmin + gmax)/2;
			if((ret = test(r, g)) == 0) {
				*py = g;
				return r;
			} else if(ret == 1) {
				gmax = g;
			} else {
				gmin = g;
			}
		}
		if(test(r, gmin) == 0) {
			*py = gmin;
			return r;
		} else if(test(r, gmax) == 0) {
			*py = gmax;
			return r;
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


	
char inbuf[2048];

int main()
{
	LL sqd, x, y, v;
#ifdef DEBUG
	while(1) {
#endif
	if(fgets(&(inbuf[0]), 2047, stdin) == NULL) {
		fprintf(stderr, "read failed on nvals\n");
		return -1;
	}
	if((sscanf(&(inbuf[0]), "%lld %lld", &p, &q)) != 2) {
		fprintf(stderr, "scan failed on counts\n");
		return -2;
	}
	if((p < 1) || (q < (2*p -1)) || (q >1000)) {
		fprintf(stderr, "p %lld < 1 OR q %lld < 2*p - 1 or q > 1000\n", p, q);
		return -3;
	}
	if((v = GCD(p, q)) > 1) {
		x = p/v;
		y = q/v;
		p = x; q = y;
		printf(" not lowest terms now %lld %lld\n", p, q);
	}
	if((p == 1) && (q == 1)) {
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


