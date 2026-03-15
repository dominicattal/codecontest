/*
 * Greater NY Regional 2022
 * Clarissas Cannolis
 * Fred Pickel
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

typedef unsigned long DWORD;
typedef unsigned char BYTE;

#define START_FLAG	1000
double M_EPS = .0001;
char inbuf[256];
const double M_PI_D = 3.1415926535897932384626433;


int main()
{
	int ret;
	double slantHt, coneDiam, diskRad, overlap, coneRad, ang, cenOff;
	if(fgets(&(inbuf[0]), 255, stdin) == NULL) {
		fprintf(stderr, "read failed on input\n");
		return -1;
	}
	if((ret = sscanf(&(inbuf[0]), "%lf %lf %lf %lf", 
		&coneDiam, &slantHt, &diskRad, &overlap)) != 4) {
		fprintf(stderr, "scan of input ret %d\n", ret);
		return -2;
	}
	coneRad = 0.5*coneDiam;
	ang = (M_PI_D*coneRad)/slantHt; // one half the central angel of the unrolled cone
	if(ang >= (0.5*M_PI_D)) { // cannot get overlap
		printf("-2.0\n");
		return 0;
	}
	if(ang < M_EPS) { // cone too skinny
		printf("-1.0\n");
		return 0;
	}
	cenOff = (diskRad - 0.5*overlap)/sin(ang);
	if(cenOff < diskRad) { // cannot get overlap
		printf("-2.0\n");
		return 0;
	}
	if((cenOff + diskRad) > slantHt) { // cone too skinny big overlap
		printf("-1.0\n");
		return 0;
	}
	printf("%0.1lf\n", slantHt - (cenOff + diskRad));
	return 0;
}
