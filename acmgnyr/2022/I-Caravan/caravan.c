/*
 * Greater NY Regional 2022
 * Caravan Trips
 * Fred Pickel
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

typedef unsigned long DWORD;
#ifdef WIN32
typedef unsigned __int64 DDWORD;
#else
typedef unsigned long long DDWORD;
#endif

int noasis, ntrips;

int oases[21], last_oasis;
DDWORD maxret = 0x7FFFFFFFFFFFFFFF;
/*
 * we need to distribute oasis_stays = days - dest rest days
 * among the oases on the route
 * if we let O mean one day resting at an oasis and
 * T mean transit from one oasis to the next (several days travel)
 * then a trip plan is a string of O's and T's containing oasis_stays O's
 * dest_ind T's for a total of(oasis_stays + dest_ind) codes
 * so the number of trip plans is the number of ways of choosing oasis_stays
 * spots out of(oasis_stays + dest_ind) locations
 */

DDWORD CountTripPlans(int dest, int days, int dest_ind)
{
	int oasis_stays;
	DDWORD lim, ret, tot, choose, i;
	ret = 1;
	lim = maxret/last_oasis;
	oasis_stays = days - dest;
	tot = oasis_stays + dest_ind;
	if(oasis_stays < dest_ind) choose = oasis_stays;
	else choose = dest_ind;	// choose(n,k) = choose(n,n-k)
	for(i = 1; i <= choose ; i++) {
		if(ret > lim) {
#ifdef WIN32
			printf("potential overflow ret %I64u, lim %I64u, tot %I64u\n",
#else
			printf("potential overflow ret %llu, lim %llu, tot %llu\n",
#endif
				ret, lim, tot);
		}
		ret *= tot;
		ret /= i;
		tot--;
	}
	return ret;
}

char inbuf[256];
int main()
{
	int ind, dest, days, dest_ind;
	DDWORD result;
	if(fgets(&(inbuf[0]), 255, stdin) == NULL) {
		fprintf(stderr, "read failed on sizes\n");
		return -1;
	}
	if(sscanf(&(inbuf[0]), "%d %d",
		&noasis, &ntrips) != 2) {
		fprintf(stderr, "scan failed on sizes\n");
		return -2;
	}
	if((noasis < 5) || (noasis > 20)) {
		fprintf(stderr, "n oases not in range 5 .. 20\n");
		return -7;
	}
	if((ntrips < 1) || (ntrips > 5)) {
		fprintf(stderr, "n trips not in range 1 .. 5\n");
		return -8;
	}
	if(fgets(&(inbuf[0]), 255, stdin) == NULL) {
		fprintf(stderr, "read failed on oases\n");
		return -3;
	}
	oases[0] = 0;
	if(sscanf(&(inbuf[0]), "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
		&oases[1], &oases[2], &oases[3], &oases[4], &oases[5],
		&oases[6], &oases[7], &oases[8], &oases[9], &oases[10],
		&oases[11], &oases[12], &oases[13], &oases[14], &oases[15],
		&oases[16], &oases[17], &oases[18], &oases[19], &oases[20]) != noasis) {
		fprintf(stderr, "scan failed on oases\n");
		return -4;
	}
	for(ind = 1; ind <= noasis ; ind++) {
		if(oases[ind-1] >= oases[ind]) {
			fprintf(stderr, "oases[%d} >= oases[%d]\n", ind - 1, ind);
			return -9;
		}
	}
	last_oasis = oases[noasis];
	for(ind = 0; ind < ntrips; ind++) {
		if(fgets(&(inbuf[0]), 255, stdin) == NULL) {
			fprintf(stderr, "read failed on trip data %d\n", ind+1);
			return -5;
		}
		if(sscanf(&(inbuf[0]), "%d %d",
			&dest_ind, &days) != 2) {
			fprintf(stderr, "scan failed on trip data %d\n", ind+1);
			return -6;
		}
		if(dest_ind > noasis) {
			fprintf(stderr, "%d: dest ind %d > last oasis %d\n",
				ind+1, dest, last_oasis);
			return -10;
		}
		if((dest = oases[dest_ind]) > days) {
			fprintf(stderr, "%d: dest %d > days %d\n",
				ind+1, dest, days);
			return -11;
		}
		result = CountTripPlans(dest, days, dest_ind);
#ifdef WIN32
		printf("%I64u\n", result);
#else
		printf("%llu\n", result);
#endif
	}
	return 0;
}
