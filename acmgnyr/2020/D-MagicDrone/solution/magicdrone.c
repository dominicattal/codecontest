/*
 * ICPC Greater NY Regional Contest, March 2020
 * Magic Drone
 * Based on Airspace Regulations (discard from NAC) - solution by Fred Pickel, C-Scape Consulting Corp
 */
#include <stdio.h>
#include <math.h>

#define MAX_REGS	2000
#define MAX_V		10.0
#define MAX_A		5.0
#define MIN_A		-5.0
#define MAX_REG		400.0
#define MAX_VAL		(1.0e9)
#define	FPM			5280
#define EPS (1.0e-10)
double regx[MAX_REGS];
double regy[MAX_REGS];
double regresult[MAX_REGS];
double V, Amin, Amax;
int nRegs;
double xcen, ymax, ybrk, vbrk, xbrk, xend, curT, curT2, Arat, miny;

/*
 * To reduce limitation from nearby restricitons the optimal path will always
 * stat at y = 0, dyd/dt = 0, accelerate at Amax for t1 seconds to ybrk = 0.5*Amax*t1^2 and dy/dt = t1*Amax
 * then decelerate at Amin for t2 = (-t1*Amax)/Amin seconds to ymax = ybrk - 0.5*Amin*t2^2 and dy/dt = 0
 * then decelerate at Amin another t2 seconds and accelerate at Amax another t1 seconds to y = 0, dy/dt = 0
 * Note that the path is symmetric around t1+t2
 * ymax = 0.5*Amax*t1^2 - 0.5*Amin*t2^2 = 0.5*Amax*t1^2 - 0.5*Amin*(-(t1*Amax)/Amin)^2
 *		= 0.5*Amax*t1^2 - 0.5*Amin*(Amax)/Amin)^2 * t1^2 = 0.5*Amax*t1^2*(1.0 - (Amax/Amin))
 *		= 0.5*Amax*t1^2*(1.0 + Arat)	(where Arat = -Amax/Amin)
 *
 * GENERAL approach:
 * For each regulation (reg) we look for the path that give the maximum y at x_reg (up to y_reg)
 * by trying paths until we find one which violates no other regulations
 * If y(x_reg) > y_reg, we can scale the acceleration to make it fit
 *
 * at each step, we try a path centered at xcen with width/height parameter t
 *	we test if this path vialoates and regualtions to the left resp right of x_reg
 *
 *	If it only violates on one side, say at x_l,y_l we choose a curve which passes through
 *		(x_l,y_l) (accelerating at Amax) and has inflection point at x_reg and go back to test
 * 
 *	If the curve violates on both sides (at (x_l,y_l) and x_r,y_r)
 */

// init curve parameters for the input t value (time from 0 to inflection point)
// these parameters are for the max Y at 0
int SetT(double t)
{
	curT = t;
	ymax = 0.5 * (Amax*(1.0 + Arat))*t*t;	// maximum y
	ybrk = 0.5*Amax*t*t;	// y at inflection point
	xbrk = t*V;				// x at inflection point
	vbrk = t*Amax;			// vertical velocity at inflection point
	curT2 = t*Arat;			// time at Amin to maximum
	xbrk = curT2*V;			// x at inflection point (from max)
	xend = (t + curT2)*V;	// x at landing (- for takeoff)
	return 0;
}

// find T value that results in a maximum at the center of maxVal
// solve ymax = = 0.5*Amax*t1^2*(1.0 + Arat) for t1
// set t and xcen
double FindTmax(double maxVal, double xmax)
{
	double t, fact = 0.5 * (Amax*(1.0 + Arat));
	t = sqrt(maxVal/fact);
	SetT(t);
	xcen = xmax;
	return t;
}

// find T value that results in an accelerates thru ybk on to inflection pt at xreg
// solve ybk = = 0.5*Amax*t1^2 then continue to xreg
// set t and xcen
double FindAccel(double xbk, double ybk, double xreg)
{
	double t, fact = 0.5 * Amax;
	t = sqrt(ybk/fact);
	if(xreg > xbk) {
		t += (xreg - xbk)/V;
	} else {
		t -= (xreg - xbk)/V;
	}
	
	SetT(t);
	if(xreg > xbk) {
		xcen = xreg + xbrk;
	} else {
		xcen = xreg - xbrk;
	}
	return t;
}

// get y value for input offset from center x
double GetY(double x)
{
	double t;
	if(x < 0.0) x = -x;	// use symmetry
	if(x >= xend) return 0.0;	// past landing /takeoff
	if(x >= xbrk) {				// x is past inflection point, accelerate up from xend at Amax
		t = (xend - x)/V;
		return (0.5*Amax*t*t);
	} else {					// x between max and inflection point, decelerate at Amin from max
		t = x/V;
		return (ymax + 0.5*Amin*t*t);
	}
}

// get xoffset from center for input y value
// which must be between 0 and ymax
double GetX(double y)
{
	double t;
	if((y > ymax) || (y < 0.0)) return -1.0;
	if(y > ybrk) {		// higher than inflection point solve y - ymax = 0.5*Amin*t^2
		t = sqrt(2.0*(y - ymax)/Amin);
		return (t*V);
	} else {		// below inflection point solve y = 0.5*Amax*t^2
		t = sqrt(2.0*y/Amax);
		return (xend - t*V);
	}
}

// for regs left of reg and path centered at xcen with parameter t
// find the reg that is most violated
// and among those satisfied, the one that is closest to the curve
// use comparison relative to the value of the constraint
// return constraint index of most violated constraint
// if no violations, return -2 - index
// return -1 some reg exactly hits
// NOTE reg > 0
int CheckViolationsLeft(int reg, double xcen, double scale)
{
	int i, imax, imin;
	double x, y, max, min, diff;
	imax = -1;
	imin = -1;
	max = -1.0;
	min = 2.0e6;
	i = reg - 1;
	while(i >= 0) {
		x = xcen - regx[i];
		if(x > xend) return imax;
		y = GetY(x)*scale;
		if((diff = (y - regy[i])) > EPS*regy[i]) {
			if(diff > max) {
				max = diff;
				imax = i;
			}
		} else if((diff = (regy[i] - y)) < min) {
			min = diff;
			imin = i;
		}
		i--;
	}
	if(imax >= 0) {
		return imax;
	} else if(min > (EPS*regy[imin])) {	// do not return exact matches
		return (-2-imin);
	} else {
		return -1;
	}
}

// for regs right of reg and path centered at xcen with parameter t
// find the reg that is most violated
// and among those satisfied, the one that is closest to the curve
// use comparison relative to the value of the constraint
// return constraint index of most violated constraint
// if no violations, return -2 - index for reg with min y_reg - y(x_reg) (if > EPS)
// return -1 some reg exactly hits
// NOTE reg < nRegs - 1
int CheckViolationsRight(int reg, double xcen, double scale)
{
	int i, imax, imin;
	double x, y, max, min, diff;
	imax = -1;
	imin = -1;
	max = -1.0;
	min = 2.0e6;
	i = reg + 1;
	while(i < nRegs) {
		x = regx[i] - xcen;
		if(x > xend) return imax;
		y = GetY(x)*scale;
		if((diff = (y - regy[i])) > EPS*regy[i]) {
			if(diff > max) {
				max = diff;
				imax = i;
			}
		} else if((diff = (regy[i] - y)) < min) {
			min = diff;
			imin = i;
		}
		i++;
	}
	if(imax >= 0) {
		return imax;
	} else if(min > (EPS*regy[imin])) {
		return (-2-imin);
	} else {
		return -1;
	}
}

// if ylft > yrt, try to find T so GetX(ylft) - GetX(yrt) = dx
// else (yrt > ylft) try to find T so GetX(yrt) - GetX(ylft) = dx
// do a secant method step every other step to speed things up
double TwoSlantT(double ylft, double yrt, double xlft, double xrt, double tmin, double tmax)
{
	double t, t1, t2, curdx, prevdx, ddx, dt, dx, xr, xl;
	int itcnt, dosec, swap = 0;;
	dx = xrt - xlft;
	t1 = tmax;
	t2 = tmin;
	SetT(0.5*(t1 + t2));
	if(yrt < ylft) {
		t = ylft;
		ylft = yrt; yrt = t;
		swap = 1;
	}
	curdx = GetX(ylft) - GetX(yrt);
	if(fabs(curdx - dx) < EPS*V) {
		xr = GetX(yrt);
		xl = GetX(ylft);
		if(swap) {
			xcen = 0.5 * ((xrt - xr) + (xlft - xl));
		} else {
			xcen = 0.5 * ((xrt + xr) + (xlft + xl));
		}
		return curT;
	}
	for(itcnt = 0; itcnt < 40 ; itcnt++){
		if((fabs(t1 - t2) < 0.1) && (itcnt & 1) && (fabs(ddx) > EPS)){	// secant step
			t = curT + (dx - curdx)*dt/ddx;
			if(t > t1) {
				t = t1 - .01*(t1-t2);
			} else if(t < t2) {
				t = t2 + .01*(t1-t2);
			}
		} else {	// linear step
			t = 0.5*(t1+t2);
		}
		dt = t - curT;
		prevdx = curdx;
		SetT(t);
		dosec = 1;
		curdx = GetX(ylft) - GetX(yrt);
		if(fabs(curdx - dx) < EPS*V) {
			xr = GetX(yrt);
			xl = GetX(ylft);
			if(swap) {
				xcen = 0.5 * ((xrt - xr) + (xlft - xl));
			} else {
				xcen = 0.5 * ((xrt + xr) + (xlft + xl));
			}
			return t;
		}
		if(curdx < dx) {
			t1 = t;
		} else {
			t2 = t;
		}
		ddx = curdx - prevdx;
	}
	return -1.0;
}

// try to find T so GetX(ylft) + GetX(yrt) = dx
// do a secant method step every other step to speed things up
double TwoSidedT(double ylft, double yrt, double xlft, double xrt, double tmin, double tmax)
{
	double t, t1, t2, curdx, prevdx, ddx, dt, dx, xl, xr;
	int itcnt;
	t1 = tmax;
	t2 = tmin;
	dx = xrt - xlft;
	SetT(0.5*(t1 + t2));
	curdx = GetX(ylft) + GetX(yrt);
	if(fabs(curdx - dx) < EPS*V) {
		xr = GetX(yrt);
		xl = GetX(ylft);
		xcen = 0.5 * ((xrt - xr) + (xlft + xl));
		return curT;
	}
	if(curdx < dx) {
		t2 = curT;
	} else {
		t1 = curT;
	}
	for(itcnt = 0; itcnt < 40 ; itcnt++){
		if((itcnt > 2) && (fabs(ddx) > EPS*V)){
			t = curT + (dx - curdx)*dt/ddx;
			if(t > t1) {
				t = t1 - .01*(t1-t2);
			} else if(t < t2) {
				t = t2 + .01*(t1-t2);
			}
		} else {
			t = 0.5*(t1 + t2);
		}
		dt = t - curT;
		prevdx = curdx;
		SetT(t);
		curdx = GetX(ylft) + GetX(yrt);
		if(fabs(curdx - dx) < EPS*V) {
			xr = GetX(yrt);
			xl = GetX(ylft);
			xcen = 0.5 * ((xrt - xr) + (xlft + xl));
			return t;
		}
		if(curdx > dx) {
			t1 = t;
		} else {
			t2 = t;
		}
		ddx = curdx - prevdx;
	}
	return -1.0;
}

/*
 * try to find T so GetX(ylft) + GetX(yrt) = dx, put xcen in *pxcen
 * for description assume ylft <= yrt
 * there are two caes
 * 1. both the the left of the max ht pt
 * 2. lft to left of max rt to rt of max ht pt
 * to decide, we try a curve with max at larger value
 * if other is above this curve, cons on both sides of max
 * if other is below, cons on same side of max
 */
double FindTX(double ylft, double yrt, double xlft, double xrt, double xreg, double *pxcen)
{
	int swap = 0;
	double tmin, xcmin, tlmax, xclmax, trmax, xcrmax, dx, temp, t;
	double tmax;
	dx = xrt - xlft;
	// first try accel thru each side constraint to reg
	// and see if it satisfies the other constraint
	trmax = FindAccel(xrt, yrt, xreg);
	xcrmax = xcen;
	temp = GetY(xcen - xlft);
	if(temp < ylft) {
		*pxcen = xcen;
		return trmax;
	}
	tlmax = FindAccel(xlft, ylft, xreg);
	xclmax = xcen;
	temp = GetY(xrt - xcen);
	if(temp < yrt) {
		*pxcen = xcen;
		return tlmax;
	}
	// to decide whether case 1 or 2 find curve with max at larger of ylft, yrt
	// if val at other < y at other, max between cons, else both on same side of max
	// split int cases on whihc y is larger 
	if(ylft < yrt) {
		tmin = FindTmax(yrt, xrt);
		xcmin = xcen;
		temp = GetY(dx);
		if(fabs(temp - ylft) < EPS*ylft) {
			*pxcen = xrt;
			return tmin;
		} else if(temp < ylft) { // cons on opposite sides of peak
			tmax = tlmax;
			if(tmax > trmax) tmax = trmax;
			t = TwoSidedT(ylft, yrt, xlft, xrt, tmin, tmax);
			*pxcen = xcen;
			return t;
		} else {	// cons to left of peak
			tmax = tlmax;
			t = TwoSlantT(ylft, yrt, xlft, xrt, tmin, tmax);
			*pxcen = xcen;
			return t;
		}
	} else {
		tmin = FindTmax(ylft, xlft);
		xcmin = xcen;
		temp = GetY(dx);
		if(fabs(temp - yrt) < EPS*yrt) {
			*pxcen = xlft;
			return tmin;
		} else if(temp < yrt) { // cons on opposite sides of peak
			tmax = tlmax;
			if(tmax > trmax) tmax = trmax;
			t = TwoSidedT(ylft, yrt, xlft, xrt, tmin, tmax);
			*pxcen = xcen;
			return t;
		} else {	// cons to rt of peak
			tmax = trmax;
			t = TwoSlantT(ylft, yrt, xlft, xrt, tmin, tmax);
			*pxcen = xcen;
			return t;
		}
	}
	return -1.0;
}
// find the curve which satisfies all other constraints and has the maximum value at
// regx[reg]. if this value is less than regy[reg] return it as the maximum
// otherwise return regy[reg]. The actual flight path will scale Amax and Amin by
// regy[reg]/ymax
double FindMaxHt(int reg)
{
	double t, txcen, yval, scale;
	int lft, rt, olft, ort, itcnt;
	olft = ort = -1;	// have we ever seen a restriction on either side
	// first just try a flight centered at regx with max of regy, if it works done
	t = FindTmax(regy[reg], regx[reg]);
	itcnt = 0;
	// loop looking for path with max ht at regx[reg]
	while(itcnt < 40) {
		scale = 1.0;
		yval = GetY(regx[reg] - xcen);
		if(yval > regy[reg]) {
			scale = regy[reg]/yval;
		}
		// curT and xcen set, test current paath for violations
		lft = CheckViolationsLeft(reg, xcen, scale);
		rt = CheckViolationsRight(reg, xcen, scale);
		if((lft < 0) && (rt < 0)) {
			if(yval >= regy[reg]) {
				return regy[reg];	// satisfies all other constraints
			} else {
				return yval;	// best we can do
			}
		}
		// if no constraints on a side and there are pevious violations, use them on next step
		if(lft < 0) {
			lft = olft;
		}
		if(rt < 0) {
			rt = ort;
		}
		// if only constrain on one side try Amax from constraint to regx
		if(lft < 0) {
			ort = rt;
			t = FindAccel(regx[rt], regy[rt], regx[reg]);
		}
		else if(rt < 0) {
			olft = lft;
			t = FindAccel(regx[lft], regy[lft], regx[reg]);
		}
		else {	// hve violations on both sides
			olft = lft;
			ort = rt;
			t = FindTX(regy[lft], regy[rt], regx[lft], regx[rt], regx[reg], &txcen);
		}
	}
	return -1.0;
}

char inbuf[256];
int main()
{
	int index, i;
	double prevx, result, rx, ry;

	if(fgets(inbuf, 256, stdin) == NULL) {
		fprintf(stderr, "read failed on reg cnt\n");
		return -1;
	}
	if(sscanf(inbuf, "%d %lf %lf %lf", &nRegs, &V, &Amin, &Amax) != 4){
		fprintf(stderr, "scan failed on probelm header\n");
		return -2;
	}
	if((nRegs < 1) || (nRegs > MAX_REGS) || (V <= 0.0) || (V > MAX_V) ||
		(Amin >= 0.0) || (Amin < MIN_A) ||(Amax <= 0.0) || (Amax > MAX_A)) {
		fprintf(stderr, "header val %d %lf %lf %lf out of range\n",
			nRegs, V, Amin, Amax);
		return -3;
	}
	/* Convert to fph then fps */
	Amax *= FPM;
	Amax /= 3600.0;
	Amin *= FPM;
	Amin /= 3600.0;
	V *= FPM;
	V /= 3600.0;
	Arat = -Amax/Amin;
	miny = MAX_VAL;
	prevx = -MAX_VAL;
	for(i = 0; i < nRegs ; i++) {
		if(fgets(inbuf, 256, stdin) == NULL) {
			fprintf(stderr, "read failed on reg %d\n", i+1);
			return -4;
		}
		if(sscanf(inbuf, "%lf %lf", &rx, &ry) != 2) {
			fprintf(stderr, "scan failed on reg %d\n", i+1);
			return -5;
		}
		if((ry <= 0.0) || (ry > MAX_REG) || (rx <= prevx)) {
			fprintf(stderr, "regulation %d %.3lf %.3lf x or y out of range (prevx %.3lf)\n",
				i+1, rx, ry, prevx);
			return -6;
		}
		/* Convert to miles */
		regx[i] = rx;
		regy[i] = ry;
		if(regy[i] < miny) miny = regy[i];
		prevx = regx[i];
	}
	for(i = 0; i < nRegs ; i++) {
		result = FindMaxHt(i);
		if(result < 0.0) {
			printf("internal error on restriction %d\n", i+1);
		} else {
			printf("%.3lf\n", result);
		}
	}
	return 0;
} 
