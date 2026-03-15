/*
 * PSET game
 * Author: Fred Pickel
 * ICPC GNYR 2022 Regional Contest
 * 
 * This is basically three nested loops + parse the input + decide if 3 cards make a pset
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

//#define TEST

#define MIN_CARD 4
#define MAX_CARD 20

char inbuf[256];
char cards[MAX_CARD][12];
char colors[4] = "RGB";
char fills[4] = "ESF";
char shapes[4] = "ADO";
char counts[4] = "123";
char used[81];

/*
	If you convert the feature codes 123 RGB ESF ADO to (feature index) 012 in each case
	then featues from 3 cards are  all the same or all different if and only if the sum
	of the feature indices is congruent to 0 mod 3
	if 0 <= a,b <= 2 then 1 <= a+1+b <= 5 so if ((a+1+b) % 3) == 0, a+1+b = 3 and a+b = 2
	thus if 2 set cards make a set with 2GSD (1111), each pair of their feature indices
	satisfy b = 2 - a
 */
/* parse one pset card input line */
int ParseCard(char *pbuf, char  *pcard)
{
	int  cnt,  clr, fill, shape, code;
	char *pb;
	/* skip white space */
	while((*pbuf != '\0') && isspace(*pbuf)) {
		pbuf++;
	}
	/* convert top feature codes to feature indices */
	if((pb = strchr(counts, pbuf[0])) == NULL) {
		return -21;
	}
	cnt  = pb - counts;
	if((pb = strchr(colors, pbuf[1])) == NULL) {
		return -22;
	}
	clr  = pb - colors;
	if((pb = strchr(fills, pbuf[2])) == NULL) {
		return -23;
	}
	fill  = pb - fills;
	if((pb = strchr(shapes, pbuf[3])) == NULL) {
		return -24;
	}
	shape  = pb - shapes;
	code = 3*(3*(3*cnt + clr) + fill) + shape;
	if(used[code] != 0) {	// check for duplicate cards
		return -40;
	}
	used[code] = 1;
	/* save top indices */
	pcard[0] = (char)(cnt & 0x7f);
	pcard[1] = (char)(clr & 0x7f);
	pcard[2] = (char)(fill & 0x7f);
	pcard[3] = (char)(shape & 0x7f);
	pbuf += 4;
	/* skip white space */
	while((*pbuf != '\0') && isspace(*pbuf)) {
		pbuf++;
	}
	/* convert bottom feature codes to feature indices */
	if((pb = strchr(counts, pbuf[0])) == NULL) {
		return -25;
	}
	cnt  = pb - counts;
	if((pb = strchr(colors, pbuf[1])) == NULL) {
		return -26;
	}
	clr  = pb - colors;
	if((pb = strchr(fills, pbuf[2])) == NULL) {
		return -27;
	}
	fill  = pb - fills;
	if((pb = strchr(shapes, pbuf[3])) == NULL) {
		return -28;
	}
	shape  = pb - shapes;
	code = 3*(3*(3*cnt + clr) + fill) + shape;
	if(used[code] != 0) {	// check for duplicate cards
		return -40;
	}
	/* save bottom indices */
	pcard[4] = (char)(cnt & 0x7f);
	pcard[5] = (char)(clr & 0x7f);
	pcard[6] = (char)(fill & 0x7f);
	pcard[7] = (char)(shape & 0x7f);
	/* check that top and bottom make a set with 2BSD (1111) */
	/* this happens if top index is 2 - bottom index */
	if(pcard[4] != (2 - pcard[0])) {
		return (-30);
	}
	if(pcard[5] != (2 - pcard[1])) {
		return (-31);
	}
	if(pcard[6] != (2 - pcard[2])) {
		return (-32);
	}
	if(pcard[7] != (2 - pcard[3])) {
		return (-33);
	}
	return 0;
}

/* see if featrue indices correspond to a set
	if any featrue indices are not congruent to 0 mod 3 it is not a set 
 */
int IsSet(char *pc1, char *pc2, char *pc3)
{
	if(((pc1[0] + pc2[0] + pc3[0]) %3) != 0) {
		return 0;
	}
	if(((pc1[1] + pc2[1] + pc3[1]) %3) != 0) {
		return 0;
	}
	if(((pc1[2] + pc2[2] + pc3[2]) %3) != 0) {
		return 0;
	}
	if(((pc1[3] + pc2[3] + pc3[3]) %3) != 0) {
		return 0;
	}
	return 1;
}

/* check if 3 double feature index sets form a PSET
	test 123, 1flip23, 12flip3 and 1flip2flip3 */
int IsPSet(char *pc1, char *pc2, char *pc3)
{
	if(IsSet(pc1, pc2, pc3)) {
		return 1;
	}
	if(IsSet(pc1, &(pc2[4]), pc3)) {
		return 1;
	}
	if(IsSet(pc1, pc2, &(pc3[4]))) {
		return 1;
	}
	if(IsSet(pc1, &(pc2[4]), &(pc3[4]))) {
		return 1;
	}
	return 0;
}

/* for each collection of 3 pset cards, check if it is a pset
 and if so increment counter */
int CountPSets(int ncards)
{
	int i, j, k, psetcnt;
	psetcnt = 0;
	for(i = 0; i < (ncards-2) ; i++) {
		for(j = i+1; j < (ncards-1) ; j++) {
			for(k = j+1; k < ncards ; k++) {
				if(IsPSet(&(cards[i][0]), &(cards[j][0]), &(cards[k][0]))) {
					psetcnt++;
				}
			}
		}
	}
	return psetcnt;
}

int main()
{
	int cardcnt, i, ret, cnt;
#ifdef TEST
	int nprob, curprob, probnum, problines;
	if(fgets(&(inbuf[0]), 255, stdin) == NULL) {
		fprintf(stderr, "read failed on problem count\n");
		return -1;
	}
	if(sscanf(&(inbuf[0]), "%d", &nprob) != 1){
		fprintf(stderr, "scan failed on problem count\n");
		return -2;
	}
	for(curprob = 1; curprob <= nprob  ;  curprob++) {
		if(fgets(&(inbuf[0]), 255, stdin) == NULL) {
			fprintf(stderr, "read failed on problem num & sz\n");
			return -3;
		}
		if(sscanf(&(inbuf[0]), "%d %d", &probnum, &problines) != 2){
			fprintf(stderr, "scan failed on num & sz\n");
			return -4;
		}
#endif
	memset(used, 0, 81);
	if(fgets(&(inbuf[0]), 255, stdin) == NULL) {
		fprintf(stderr, "read failed on card count\n");
		return -5;
	}
	if(sscanf(&(inbuf[0]), "%d", &cardcnt) != 1){
		fprintf(stderr, "scan failed on card count\n");
		return -6;
	}
	if((cardcnt < MIN_CARD) || (cardcnt > MAX_CARD)) {
		fprintf(stderr, "card count %d not in range %d ... %d\n", cardcnt, MIN_CARD, MAX_CARD);
		return -7;
	}
	for(i = 0; i < cardcnt; i++) {
		if(fgets(&(inbuf[0]), 255, stdin) == NULL) {
			fprintf(stderr, "read failed on card %d\n", i+1);
			return -8;
		}
		if((ret = ParseCard(&(inbuf[0]), &(cards[i][0]))) != 0) {
			fprintf(stderr, "parse of  card  %d returned  %d\n", i, ret);
			return -9;
		}
	}
	cnt = CountPSets(cardcnt);
#ifdef TEST
		printf("%d: %d  %d\n", curprob, cardcnt, cnt);
	}
#else
	printf("%d\n", cnt);
#endif
	return 0;
}
