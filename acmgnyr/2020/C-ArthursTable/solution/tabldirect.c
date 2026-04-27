/*
 * Arthur's Round Table
 * Author: Fred Pickel
 * ICPC GNYR 2020 Regional Contest
 *
 * I think this is hard
 * there are two solutions
 * one uses inversion in a cricle which takes a llot of formula fiddling
 * one just computes it numerically 
 * 	guess a radius find all the circles numercally, adjust radius
 * both of these took me a while to get all the bugs out
 */
#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#define MAX_CIRCS	40
#define EPS (1.0e-8)
#define MA_PI	(3.14159265359)
//#define TEST
//#define FORMAT_1	"%0.8lf\n"
//#define FORMAT_3	"%0.8lf %0.8lf %0.8lf\n"
#define FORMAT_1	"%0.3lf\n"
#define FORMAT_3	"%0.3lf %0.3lf %0.3lf\n"

typedef struct _dpoint_
{
	double dpx;
	double dpy;
} DPOINT, *PDPOINT;

typedef struct _dcirc_ 
{
	double dcx;
	double dcy;
	double drad;
} DCIRC, *PDCIRC;

DCIRC outCirc, inCirc, prevCirc, newCirc, circs[MAX_CIRCS];
double mat[3][4];
double circrad;

/* starting with king arthur's trencher we want to find the radius and center of each
 * trancher in counter-clockwise order we start with the outer circle (table),
 * the inner circle (seving platter and the current trancher which is tangent
 * to both the inner and outer circle
 * the center (x,y0 and radius r of the next circle must satsfy 3 equations
 * testing tangency to each of the first 3 circles
 * in each case expected distance betwen centers must = actual disatance
 * outer: (outr - r)^2 = (x - outx)^2 + (y - outy)^2
 * inner: (inr + r)^2 = (x - inx)^2 + (y - iny)^2
 * previous: (prevr + r)^2 = (x - prevx)^2 + (y - prevy)^2
 */
/* outer equation actual - expected  ?= 0 */
double Fout(DCIRC outCirc, DCIRC newCirc)
{
	double xsq, ysq, rsq, tmp;
	tmp = outCirc.dcx - newCirc.dcx;
	xsq = tmp*tmp;
	tmp = outCirc.dcy - newCirc.dcy;
	ysq = tmp*tmp;
	tmp = outCirc.drad - newCirc.drad;
	rsq = tmp*tmp;
	return (xsq + ysq - rsq);
}

/* inner equation actual - expected  ?= 0 */
double Fin(DCIRC inCirc, DCIRC newCirc)
{
	double xsq, ysq, rsq, tmp;
	tmp = inCirc.dcx - newCirc.dcx;
	xsq = tmp*tmp;
	tmp = inCirc.dcy - newCirc.dcy;
	ysq = tmp*tmp;
	tmp = inCirc.drad + newCirc.drad;
	rsq = tmp*tmp;
	return (xsq + ysq - rsq);
}

/* previous equation actual - expected  ?= 0 */
double Fprv(DCIRC prevCirc, DCIRC newCirc)
{
	double xsq, ysq, rsq, tmp;
	tmp = prevCirc.dcx - newCirc.dcx;
	xsq = tmp*tmp;
	tmp = prevCirc.dcy - newCirc.dcy;
	ysq = tmp*tmp;
	tmp = prevCirc.drad + newCirc.drad;
	rsq = tmp*tmp;
	return (xsq + ysq - rsq);
}

/* to solve the 3 equations in 3 unknowns we use (3D) newtons method
 * (D_? = partial derivative wrt ?)
 *	D_x(Fout)*dx + D_y(Fout*dy + D_r(Fout)*dr = -Fout
 *	D_x(Fin)*dx + D_y(Fin*dy + D_r(Fin)*dr = -Fin
 *	D_x(Fprv)*dx + D_y(Fprv*dy + D_r(Fprv)*dr = -Fprv
 * solve for dx, dy and dr and the next guess is x+dx, y+dy, r+dr
 */
void makeMat(DCIRC outCirc, DCIRC inCirc, DCIRC prevCirc, DCIRC newCirc)
{
	mat[0][0] = -2.0*(outCirc.dcx - newCirc.dcx);
	mat[0][1] = -2.0*(outCirc.dcy - newCirc.dcy);
	mat[0][2] = 2.0*(outCirc.drad - newCirc.drad);
	mat[0][3] = -Fout(outCirc, newCirc);

	mat[1][0] = -2.0*(inCirc.dcx - newCirc.dcx);
	mat[1][1] = -2.0*(inCirc.dcy - newCirc.dcy);
	mat[1][2] = -2.0*(inCirc.drad + newCirc.drad);
	mat[1][3] = -Fin(inCirc, newCirc);

	mat[2][0] = -2.0*(prevCirc.dcx - newCirc.dcx);
	mat[2][1] = -2.0*(prevCirc.dcy - newCirc.dcy);
	mat[2][2] = -2.0*(prevCirc.drad + newCirc.drad);
	mat[2][3] = -Fprv(prevCirc, newCirc);
}

/* solve the 3 linear equations in 3 unknowns with full pivoting just in case */
int SolveMat()
{
	int pivrow, pivcol, used[3], i , j, k;
	double max, tmp;
	for(i = 0; i < 3; i++) {
		used[i] = 0;
	}
	for(k = 0; k < 3; k++)
	{
		max = -1.0;
		pivrow = pivcol = -1;
		for(i = 0; i < 3; i++) {
			if(used[i] == 0) {
				for(j = 0; j < 3; j++) {
					if(used[j] == 0) {
						if((tmp = fabs(mat[i][j])) > max) {
							max = tmp;
							pivrow = i;
							pivcol = j;
						}
					}
				}
			}
		}
		if(max < EPS) {
			return (-1 - k);
		}
		if(pivrow != pivcol) {	// swap pivrow to pivcol
			for(i = 0; i < 4; i++) {
				tmp = mat[pivrow][i];
				mat[pivrow][i] = mat[pivcol][i];
				mat[pivcol][i] = tmp;
			}
		}
		tmp = 1.0/mat[pivcol][pivcol];
		mat[pivcol][pivcol] = 1.0;
		for(i = 0; i < 4; i++) {	// divide ppivrow vy pivot
			if(i != pivcol) {
				mat[pivcol][i] *= tmp;
			}
		}
		// subtract multiples of pivrow from other rws to elimiate elts in pivcol
		for(i = 0; i < 3; i++) {
			if(i != pivcol) {
				tmp = mat[i][pivcol];
				for(j = 0; j < 4; j++) {
					mat[i][j] -= mat[pivcol][j]*tmp;
				}
			}
		}
		used[pivcol] = 1;
	}
	return 0;
}

/* find next circle counter clcokwise using newton's method
 * the inital guess uses the previous radius ans puts the center on a ray
 * form the outer circle center rotated from the preious center by the angle
 * subtanded byby the previous circle (roungly at didtance from the outer center
 * out-r - new_r
 */
int FindNextCirc(DCIRC outCirc, DCIRC inCirc, DCIRC prevCirc, PDCIRC pNewCirc)
{
	DCIRC newCirc;
	double ang, dang, r, err;
	int ret, count = 0;
	ang = atan2(prevCirc.dcy  - outCirc.dcy, prevCirc.dcx - outCirc.dcx);
	r = (prevCirc.dcx  - outCirc.dcx)*(prevCirc.dcx  - outCirc.dcx);
	r += (prevCirc.dcy - outCirc.dcy)*(prevCirc.dcy - outCirc.dcy);
	r = sqrt(r);
	dang = atan2(prevCirc.drad, r);
	ang += 2.0*dang;
	newCirc.drad = prevCirc.drad;
	r = outCirc.drad - prevCirc.drad;
	newCirc.dcx = r*cos(ang);
	newCirc.dcy = r*sin(ang);
	makeMat(outCirc, inCirc, prevCirc, newCirc);
	err = fabs(mat[0][3]) + fabs(mat[1][3]) + fabs(mat[2][3]);
	while((count < 20) && (err > EPS)) {
		if((ret = SolveMat()) != 0) {
			return ret;
		}
		newCirc.dcx += mat[0][3];
		newCirc.dcy += mat[1][3];
		newCirc.drad += mat[2][3];
		makeMat(outCirc, inCirc, prevCirc, newCirc);
		err = fabs(mat[0][3]) + fabs(mat[1][3]) + fabs(mat[2][3]);
	}
	if(count >= 20) {
		return -115;
	}
	*pNewCirc = newCirc;
	return 0;
}

/* find cirCnt addtional circles starting with prevCirc (artur) */
int FindCircs(int circCnt)
{
	int i, ret;
	if(circCnt >= MAX_CIRCS) {
		return -9;
	}
	circs[0] = prevCirc;
	for(i = 0; i < circCnt ; i++) {
		ret = FindNextCirc(outCirc, inCirc, circs[i], &(circs[i+1]));
		if(ret != 0) {
			return (ret - 10*(i+1));
		}
	}
	return 0;
}

/* Find the radius of the inner circle when theouter cicrle has radius 1
 * and the inner and outer circles are concentric
 * if r is radius of the circle between the inner and outer circle
 * then the angle between the line to a point of tangency og two such circles and
 * the line to the center of one of them is (2*PI)/(2*ncircs) and the distance o the center
 * is (1.0 - r) so
 * s = sin(PI/ncircs) = r/(1 - r) and r = s/(1 + s) and the inner circel has radius
 * 1 - 2*r
 */
double GetBaseRadRatio(int nCircs)
{
	double s = sin(MA_PI/((double)nCircs));
	circrad = s/(1.0 + s);
	return (1.0 - 2.0*circrad);
}

/* find the inner circle radius and the center and radius of the trenchers
 * as a first guess we use the radius of the inner circle when the centers of the
 * inner and outer circles coincide
 * arthur's trnacher then has radius Ra= out (rad - offset - inrad)/2 and
 * center (outrad - Ra, 0)
 * we then repeatedly compute the other circles and adjust inrad until
 * the(nCircs/2) circle is in the correct position
 * if nCircs is even, the center of that circle should be on the x axis
 * otherwise the bottom of that circl should be on the x axis
 * circ-cen_y - circ_radius = 0)
 * if the value is positive, we need to make the circle smaller and if tis is
 * negative, we need to make it larger
 * we keep track of the smallest possitive and largest negative error
 * until we have both, the next guess is inrad - 28error/(PI + nCirc)
 * (if we decrease inrad by dr we decrease the total amount the smaller radii
 * have to add up to by (dr/2)*PI and increase each radius by dr/2 and there
 * nCirc radii around the half circle)
 * after we have a positive and negative error we use the secant method but
 * use the midpoint if the secant estimate is outside the expected interval
 */
int FindOffsetCircs(double outrad, int nCircs, double offset)
{
	int count, circcnt, ret;
	double inrad, newrad, err, maxrad, maxerr, minrad, minerr, lastrad, lasterr;
	circcnt = nCircs/2;
	outCirc.dcx = outCirc.dcy = 0.0; outCirc.drad = outrad;
	inrad = outrad * GetBaseRadRatio(nCircs);
	inCirc.dcx = offset; inCirc.dcy = 0.0; inCirc.drad = inrad;
	prevCirc.dcx = outrad*(1.0 - circrad); prevCirc.dcy = 0.0; 
	prevCirc.drad = (outCirc.drad - inCirc.dcx - inrad)/2.0;
	if((ret = FindCircs(circcnt)) != 0) {
		return ret;
	}
	if((nCircs & 1) != 0) {	// odd take last cen y - radius
		err = circs[circcnt].dcy - circs[circcnt].drad;
	} else {	// evven use last circ center
		err = circs[circcnt].dcy;
	}
	if(err > 0.0) {
		maxerr = err;
		maxrad = inrad;
		minerr = -1000.0;
		minrad = 1001.0;
	} else {
		minerr = err;
		minrad = inrad;
		maxerr = 1001.0;
		maxrad = 1001.0;
	}
	count = 0;
	while((count < 20) && (fabs(err) > EPS)) {
		count++;
		if((maxrad > 1000.0) || (minrad > 1000.0)) {
			newrad = inrad - 2.0*(err/(MA_PI +((double)nCircs)));
		} else {
			if(fabs(err - lasterr) >EPS) {
				newrad = inrad - err*(inrad - lastrad)/(err - lasterr);
				if((newrad > maxrad) || (newrad < minrad)) {
					newrad = 0.5*(maxrad + minrad);
				}
			} else {
				newrad = 0.5*(maxrad + minrad);
			}
		}
		lastrad = inrad;
		lasterr = err;
		inCirc.drad = inrad = newrad;
		prevCirc.drad = (outCirc.drad - inCirc.dcx - inrad)/2.0;
		prevCirc.dcx = outCirc.drad - prevCirc.drad;
		if((ret = FindCircs(circcnt)) != 0) {
			return ret;
		}
		if((nCircs & 1) != 0) {	// odd take last cen y - radius
			err = circs[circcnt].dcy - circs[circcnt].drad;
		} else {	// evven use last circ center
			err = circs[circcnt].dcy;
		}
		if(err > 0.0) {
			if(err < maxerr) {
				maxerr = err;
				maxrad = inrad;
			}
		} else {
			if(err > minerr) {
				minerr = err;
				minrad = inrad;
			}
		}
	}
	if(err > EPS) {
		return -101;
	}
	return 0;
}

char inbuf[256];

int main()
{
	int nCirc, ret;
	double outrad, offset, diam, inrad, tst;
#ifdef TEST
	int p, sz, prob;
	p = 0;
	while(1) {
		p++;
		printf("%d:\n", p);
	if(fgets(&(inbuf[0]), 255, stdin) == NULL) {
		fprintf(stderr, "read failed on prob header\n");
		return -1;
	}
	if(sscanf(&(inbuf[0]), "%d %d", &prob, &sz) != 2){
		fprintf(stderr, "scan failed on problem header\n");
		return -2;
	}
	if(p != prob) {
		fprintf(stderr, "prob label %d != expected %d\n", prob, p);
		return -2;
	}

#endif
	if(fgets(&(inbuf[0]), 255, stdin) == NULL) {
		fprintf(stderr, "read failed on input data\n");
		return -3;
	}
	if(sscanf(&(inbuf[0]), "%lf %d %lf", &diam, &nCirc, &offset) != 3){
		fprintf(stderr, "scan failed on problem data\n");
		return -4;
	}
	outrad = 0.5*diam;
	inrad = outrad * GetBaseRadRatio(nCirc);
	tst = outrad*circrad;
	if((diam < 8.0) || (diam > 30.0)) {
		fprintf(stderr, "table diameter %lf not in range 8 ... 30\n", outrad);
		return -5;
	}
	if((nCirc < 6) ||(nCirc > MAX_CIRCS)) {
		fprintf(stderr, "num knights %d not in range 6 ... %d\n", nCirc, MAX_CIRCS);
		return -6;
	}
	if((offset < 0.1) || (offset > (tst))) {
		fprintf(stderr, "offset %lf not in range 0.1 to an original trencher radius %lf\n",
			offset, tst);
		return -7;
	}
	if((ret = FindOffsetCircs(outrad, nCirc, -offset)) != 0) {
		return ret;
	}
	printf(FORMAT_1, inCirc.drad);
	printf(FORMAT_3, circs[0].dcx, circs[0].dcy, circs[0].drad);
	printf(FORMAT_3, circs[1].dcx, circs[1].dcy, circs[1].drad);
	printf(FORMAT_3, circs[2].dcx, circs[2].dcy, circs[2].drad);
	printf(FORMAT_3, circs[3].dcx, circs[3].dcy, circs[3].drad);
#ifdef TEST
	}
#endif
	return 0;
}
