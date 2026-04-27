/*
 * Three Triangles Area
 * Author: Fred Pickel
 * ICPC GNYR 2020 Regional Contest
 *
 * Nerdy Geometry.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

//#define TEST
#define EPS	(1.0e-5)

double Bx, Cx, Cy;
double Area, AreaPrime, AreaPPrime;
double lineEqs[6][3];	// Mx + Ny = L for alt a,b,c the perpbi a,b,c
double PrimeVerts[6];	// Apx,Apy,Bpx,Bpy,Cpx,Cpy
double PPrimeVerts[6];	// Appx,Appy,Bppx,Bppy,Cppx,Cppy

// solve 2 eqns in 2 unknowns
int Solve2 (double *eq1, double*eq2, double *px, double *py)
{
	double det;
	det = eq1[0]*eq2[1] - eq1[1]*eq2[0];
	if(fabs(det) < EPS) {
		return -1;
	}
	*px = (eq1[2]*eq2[1] - eq1[1]*eq2[2])/det;
	*py = (eq1[0]*eq2[2] - eq1[2]*eq2[0])/det;
	return 0;
}

int GetLineEqs()
{
	// for c perp eq is x = ? alt ?cx perp ? = bx/2
	lineEqs[2][0] = 1.0; lineEqs[2][1] = 0.0; lineEqs[2][2] = Cx;
	lineEqs[5][0] = 1.0; lineEqs[5][1] = 0.0; lineEqs[5][2] = 0.5*Bx;
	// for B eq is Cx*x + Cy*y = ?
	lineEqs[1][0] = Cx; lineEqs[1][1] = Cy; lineEqs[1][2] = Cx*Bx;
	lineEqs[4][0] = Cx; lineEqs[4][1] = Cy; lineEqs[4][2] = 0.5*(Cx*Cx + Cy*Cy);
	// for A eq is ((Cx-Bx)*x + Cy*y = ?
	lineEqs[0][0] = Cx - Bx; lineEqs[0][1] = Cy; lineEqs[0][2] = 0.0;
	lineEqs[3][0] = Cx - Bx; lineEqs[3][1] = Cy; 
	lineEqs[3][2] = 0.5 *(Cx*Cx - Bx*Bx + Cy*Cy);
	//Ap = hc inter pb
	if(Solve2(&(lineEqs[2][0]), &(lineEqs[4][0]),
		&(PrimeVerts[0]), &(PrimeVerts[1])) != 0) {
		return -1;
	}
	//Bp = ha inter pc
	if(Solve2(&(lineEqs[0][0]), &(lineEqs[5][0]),
		&(PrimeVerts[2]), &(PrimeVerts[3])) != 0) {
		return -2;
	}
	//Cp = hb inter pa
	if(Solve2(&(lineEqs[1][0]), &(lineEqs[3][0]),
		&(PrimeVerts[4]), &(PrimeVerts[5])) != 0) {
		return -3;
	}
	return 0;
}

int GetReflect()
{
	double norm, px, py, dot;
	// App is reflection of Ap in side b (vector in dir of b is <Cx,Cy>
	norm = (Cx*Cx + Cy*Cy);
	if(norm < EPS ) {
		return -4;
	}
	px = Cx/norm; py = Cy/norm;
	// d = project b on c = b dot c *c /|c|^2
	// b - d = proj b perp c ot reflect take d - projb perp c
	// = d - (b-d) = 2*d - b
	dot = PrimeVerts[0]*px + PrimeVerts[1]*py;
	PPrimeVerts[0] = 2.0*dot*Cx - PrimeVerts[0];
	PPrimeVerts[1] = 2.0*dot*Cy - PrimeVerts[1];
	// Bpp is reflection of Bp in side c (vector in dir of c is <Bx,0>
	norm = (Bx*Bx);
	if(norm < EPS ) {
		return -5;
	}
	px = Bx/norm; py = 0.0;
	// d = project b on c = b dot c *c /|c|^2
	// b - d = proj b perp c ot reflect take d - projb perp c
	// = d - (b-d) = 2*d - b
	dot = PrimeVerts[2]*px + PrimeVerts[3]*py;
	PPrimeVerts[2] = 2.0*dot*Bx - PrimeVerts[2];
	PPrimeVerts[3] = -PrimeVerts[3];
	// Cpp is reflection of Cp in side a (vector in dir of a is <Cx-Bx,Cy>
	norm = ((Cx-Bx)*(Cx-Bx) + Cy*Cy);
	if(norm < EPS ) {
		return -6;
	}
	px = (Cx-Bx)/norm; py = Cy/norm;
	// since side a does not go thru 0, offset everything to (Bx,0) and add it back in
	// so b = Cp - B
	// d = project b on c = b dot c *c /|c|^2
	// b - d = proj b perp c ot reflect take d - projb perp c
	// = d - (b-d) = 2*d - b
	dot = (PrimeVerts[4]- Bx)*px + PrimeVerts[5]*py;
	PPrimeVerts[4] = 2.0*dot*(Cx-Bx) - (PrimeVerts[4] - Bx) + Bx;
	PPrimeVerts[5] = 2.0*dot*Cy - PrimeVerts[5];
	return 0;
}

// area = 0.5*mag((p = A->C) cross (q = A->B))
int GetAreas()
{
	double px, py, qx, qy;
	px = Cx; py = Cy; qx = Bx; qy = 0.0;
	Area = 0.5*fabs(px*qy - qx*py);
	px = PrimeVerts[4] - PrimeVerts[0]; py = PrimeVerts[5] - PrimeVerts[1];
	qx = PrimeVerts[2] - PrimeVerts[0]; qy = PrimeVerts[3] - PrimeVerts[1];
	AreaPrime = 0.5*fabs(px*qy - qx*py);
	px = PPrimeVerts[4] - PPrimeVerts[0]; py = PPrimeVerts[5] - PPrimeVerts[1];
	qx = PPrimeVerts[2] - PPrimeVerts[0]; qy = PPrimeVerts[3] - PPrimeVerts[1];
	AreaPPrime = 0.5*fabs(px*qy - qx*py);
	return 0;
}

char inbuf[256];

int main()
{
	int ret;
#ifdef TEST
	while(1) {
#endif
	if(fgets(&(inbuf[0]), 255, stdin) == NULL) {
		fprintf(stderr, "read failed on input data\n");
		return -2;
	}
	if(sscanf(&(inbuf[0]), "%lf %lf %lf", &Bx, &Cx, &Cy) != 3){
		fprintf(stderr, "scan failed on problem data\n");
		return -3;
	}
	if((ret = GetLineEqs()) != 0) {
		fprintf(stderr, "GetLineEqs() ret %d\n", ret);
		return ret;
	}
#ifdef TEST1
	printf("%lf %lf %lf %lf %lf %lf\n",
		PrimeVerts[0], PrimeVerts[1], PrimeVerts[2],
		PrimeVerts[3], PrimeVerts[4], PrimeVerts[5]);
#endif
	if((ret = GetReflect()) != 0) {
		fprintf(stderr, "GetReflect() ret %d\n", ret);
		return ret;
	}
#ifdef TEST1
	printf("%lf %lf %lf %lf %lf %lf\n",
		PPrimeVerts[0], PPrimeVerts[1], PPrimeVerts[2],
		PPrimeVerts[3], PPrimeVerts[4], PPrimeVerts[5]);
#endif
	GetAreas();
#ifdef TEST
	printf("%0.4lf %0.4lf %0.4lf\n", Area, AreaPrime, AreaPPrime);
	}
#else
	printf("%0.4lf %0.4lf %0.4lf\n", Area, AreaPrime, AreaPPrime);
#endif
	return 0;
}

