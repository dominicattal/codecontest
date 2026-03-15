/*
 * Greater NY Regional 2022
 * Linked Triangle
 * Fred Pickel
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

double EPS = 1.0e-5;
double inverts[6][3];
double v0diff[6][3];
double triperp[3];
double tridots[6];
double array[3][5];
double singpt[3];
double opppts[2][3];
double planepts[2][3];
double singdot, oppdot[2];
int trinds[6];

int nlinks;
int linkverts[10][3];

// solve 3 equations in 3 unknowns for two different RHS simultaneously
int SolveArray(int i1, int i2)
{
	int i, j, k;
	double piv, cmp, tst;
	for(i = 0; i < 3; i++) {
		for(j = i, k = -1, cmp = 0.0; j < 3; j++) {
			if((tst = fabs(array[j][i])) > cmp) {
				cmp = tst;
				k = j;
			}
		}
		if(cmp < EPS) {
			fprintf(stderr, " solve failed col %d tri 1 %d %d\n", i, i1, i2);
			return -5;
		}
		if(k != i) {	// swap to row i
			for(j = i; j < 5; j++) {
				tst = array[i][j]; array[i][j] = array[k][j]; array[k][j] = tst;
			}
		}
		tst = 1.0/array[i][i];
		array[i][i] = 1.0;
		for(j = i+1; j < 5; j++) {
			array[i][j] *= tst;
		}
		// eliminate from remaining rows
		for(j = 0; j < 3; j++) {
			if(j == i) {
				continue;
			}
			piv = array[j][i];
			array[j][i] = 0.0;
			for(k = i+1; k < 5; k++) {
				array[j][k] -= piv*array[i][k];
			}
		}
	}
	return 0;
}

// 2-norm of a 3 vestor
double norm(double *pin)
{
	double ret;
	int i;
	for(i = 0, ret = 0.0; i < 3; i++) {
		ret += pin[i]*pin[i];
	}
	ret = sqrt(ret);
	return ret;
}

// 3 vector do product
double dot(double* pin1, double*pin2)
{
	double ret;
	int i;
	for(i = 0, ret = 0.0; i < 3; i++) {
		ret += pin1[i]*pin2[i];
	}
	return ret;
}

// 3 vector cross product
int cross(double* pin1, double*pin2, double *pout)
{
	pout[0] = pin1[1]*pin2[2] - pin1[2]*pin2[1];
	pout[1] = pin1[2]*pin2[0] - pin1[0]*pin2[2];
	pout[2] = pin1[0]*pin2[1] - pin1[1]*pin2[0];
	if(norm(pout) < EPS) {
		return -1;
	}
	return 0;
}

// v0diffs is vector form base vertex (vert 0) to each other vertex
int Setup()
{ int i, j;
	for(i = 0; i < 6; i++) {
		for(j = 0; j < 3; j++) {
			v0diff[i][j] = inverts[i][j] - inverts[0][j];
		}
	}
	return 0;
}

// for each possible triple 1 i j in lexicographical order
// determine the 1 i j triangle and the remaining vertices triangle are linked
// if so add to the list of links and increment count
// NOTE vertex numbers are 0 based and arre incremented in the output lline in main
int FindLinks()
{
	int i, j, k, m, n, in1, in2, nsign, ndot;
	double c1, c2;
	nlinks = 0;
	for(i = 1; i < 5; i++) {
		for(j = i+1; j < 6; j++) {
			// get a vector perpendicular to the plane of the 1 i j triangle
			if(cross(&(v0diff[i][0]), &(v0diff[j][0]), &(triperp[0])) != 0) {
				fprintf(stderr, "degen tri 1, %d, %d\n", i+1, j+1);
				return -3;
			}
			// for each rem pt ,take dot prod of vect from v0 to vert
			// with perp and count the number of negative dot prods
			for(k = 1, nsign = ndot = 0; k < 6; k++) {
				if((k == i) || (k == j)) {
					continue;
				}
				tridots[ndot] = dot(&(v0diff[k][0]), &(triperp[0]));
				trinds[ndot] = k;
				if(fabs(tridots[ndot]) < EPS) {
					fprintf(stderr, "4 pts in plane 1, %d, %d, %d\n", i+1, j+1, k+1);
					return -4;
				}
				if(tridots[ndot] < 0.0) {
					nsign++;
				}
				ndot++;
			}
			if((nsign == 0) || (nsign == 3)) {
				continue;	// all other points on the same side
			}
			// if 1 or 2 is negative, points on either side of triangle plane
			// set singpt to point by itself on one side and opppts[i] to the other 2
			// keep track of dot prod of each - v0 with perp
			if(nsign == 1) {
				for(k = m = 0; k < 3; k++) {
					if(tridots[k] < 0.0) {
						for(n = 0; n < 3; n++) {
							singpt[n] = inverts[trinds[k]][n];
						}
						singdot = -tridots[k];
					} else {
						for(n = 0; n < 3; n++) {
							opppts[m][n] = inverts[trinds[k]][n];
						}
						oppdot[m] = tridots[k];
						m++;
					}
				}
			} else {
				for(k = m = 0; k < 3; k++) {
					if(tridots[k] > 0.0) {
						for(n = 0; n < 3; n++) {
							singpt[n] = inverts[trinds[k]][n];
						}
						singdot = tridots[k];
					} else {
						for(n = 0; n < 3; n++) {
							opppts[m][n] = inverts[trinds[k]][n];
						}
						oppdot[m] = -tridots[k];
						m++;
					}
				}
			}
			// find pts were each line from singpt to opppt hits triangle plane
			// (1/(singdot + oppdot))*(oppdot*singpt + singdot*opppt)
			for(k = 0; k < 2; k++) {
				c1 = (singdot + oppdot[k]);
				c1 = 1.0/c1;
				c2 = singdot*c1;
				c1 = oppdot[k] * c1;
				for(m = 0; m < 3; m++) {
					planepts[k][m] = (c1*singpt[m] + c2*opppts[k][m]);
				}
			}
			// set up eqns to solve for coeffs of rep of vectors from singpt to opppts
			// in terms of vectors from singpt to original tri verts
			for(k = 0; k < 3; k++) {
				array[k][0] = inverts[0][k] - singpt[k];
				array[k][1] = inverts[i][k] - singpt[k];
				array[k][2] = inverts[j][k] - singpt[k];
				array[k][3] = planepts[0][k] - singpt[k];
				array[k][4] = planepts[1][k] - singpt[k];
			}
			// solve
			if(SolveArray(i, j) != 0) {
				return -5;
			}
			// if all coeffs are positive plane pt is inside tri
			// otherwise it si outside for link, we need one in and one out
			for(k = 0, in1 = in2= 1; k < 3; k++) {
				if(array[k][3] < 0.0) {
					in1 = 0;
				}
				if(array[k][4] < 0.0) {
					in2 = 0;
				}
			}
			if((in1 ^ in2) == 0){
				continue;	// both in or both out
			}
			// we have a link, remember it
			linkverts[nlinks][0] = i;
			linkverts[nlinks][1] = j;
			nlinks++;
		}
	}
	return 0;
}

char inbuf[256];
int main()
{
	int vert, i, ret;
	for(vert = 0; vert < 6; vert++) {
		if(fgets(&(inbuf[0]), 255, stdin) == NULL) {
			fprintf(stderr, "read failed on vertex %d\n", vert + 1);
			return -1;
		}
		if(sscanf(&(inbuf[0]), "%lf %lf %lf",
			&(inverts[vert][0]), &(inverts[vert][1]), &(inverts[vert][2])) != 3) {
			fprintf(stderr, "scan failed on vertex %d\n", vert + 1);
			return -2;
		}
	}
	Setup();
	if((ret = FindLinks()) != 0) {
		return ret;
	}
	printf("%d\n", nlinks);
	for(i = 0; i < nlinks; i++) {
		printf("%d %d\n", linkverts[i][0]+1, linkverts[i][1]+1);
	}
	return 0;
}
