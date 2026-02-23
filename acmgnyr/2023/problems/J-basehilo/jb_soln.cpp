/*
 * bhilo_jb.cpp : Solution to interactve problem of guessing arbritray based value
 * John Buck, Greater New York Chief Judge
 */
//#define _DEBUG
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define	CORRECT_ANSWER	"correct"
#define	CHEATER_ANSWER	"cheater"

#define	MAX_NUM_DIGS	64
char testcase[MAX_NUM_DIGS + 1];
char lastguess[MAX_NUM_DIGS + 1];
int testdigits[MAX_NUM_DIGS];
int maxdigits[MAX_NUM_DIGS];
int mindigits[MAX_NUM_DIGS];
char response[MAX_NUM_DIGS + 10];


const char *szDigs = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

void send_guess(int nLen)
{
	int i;
	char c;

	for(i = 0; i < nLen; i++){
		c = szDigs[testdigits[i]];
		lastguess[i] = c;
		putc(c, stdout);
	}
	lastguess[i] = '\0';
	putc('\n', stdout);
	fflush(stdout);
#ifdef _DEBUG
	fprintf(stderr, "Sent guess \"%s\" to judge\n", &(lastguess[0]));
#endif
}

void cheater()
{
	char szLine[32];
	char fmtbuf[32];

	fprintf(stdout, "%s\n", CHEATER_ANSWER);
	fflush(stdout);
	snprintf(&(fmtbuf[0]), sizeof(fmtbuf), "%%%lus", sizeof(szLine-1));
	/* Need to wait for response */
	scanf(&(fmtbuf[0]), &(szLine[0]));
#ifdef _DEBUG
	fprintf(stderr, "Got back %s from judge after telling him he was cheating\n", &(szLine[0]));
#endif
}

void check_case(int nc, int base)
{
	int nLen, i, nDig;
	char fmtbuf[32];

	if(scanf("%d", &(nLen)) != 1){
		fprintf(stderr, "Error reading length for case %d\n", nc);
		return;
	}
#ifdef _DEBUG
	fprintf(stderr, "check_case %d - len=%d\n", nc, nLen);
#endif
	if(nLen > MAX_NUM_DIGS){
		fprintf(stderr, "Too many digits requested %d, max is %d\n", nLen, MAX_NUM_DIGS);
		return;
	}
	/* Form initial guess */
	for(i = 0; i < nLen; i++){

		testdigits[i] = base / 2;
		maxdigits[i] = base - 1;
		mindigits[i] = 0;
	}
	/* Enough for "correct" */
	::snprintf(&(fmtbuf[0]), sizeof(fmtbuf), "%%%ds", nLen + 8);
	for(;;){
		send_guess(nLen);
		/* Read response */
		if(scanf(&(fmtbuf[0]), &(response[0])) != 1){
			fprintf(stderr, "Bad read on response from judge\n");
			return;
		}
#ifdef _DEBUG
		fprintf(stderr, "Got \"%s\" from judge for guess \"%s\"\n", &(response[0]), &(lastguess[0]));
#endif
		if(strcmp(&(response[0]), CORRECT_ANSWER) == 0){
			return;
		}
		if(::strlen(&(response[0])) != nLen){
			fprintf(stderr, "Bad response length from judge response: \"%s\"\n", &(response[0]));
			return;
		}

		for(i = 0; i < nLen; i++){
			if(response[i] != '='){
				break;
			}
		}
		if(i >= nLen){
				cheater();
				return;
		}

		for(i = 0; i < nLen; i++){
			if(response[i] == '-'){
				nDig = testdigits[i] - 1;
				if(nDig < mindigits[i]){
					cheater();
					return;
				}
				maxdigits[i] = nDig;
				if(maxdigits[i] < 0){
					maxdigits[i] = 0;
				}
			} else if(response[i] == '+'){
				nDig = testdigits[i] + 1;
				if(nDig > maxdigits[i]){
					cheater();
					return;
				}
				mindigits[i] = nDig;
				if(mindigits[i] >= base){
					mindigits[i] = base - 1;
				}
			} else {
				/* Is there a cheat case here?? */
				nDig = testdigits[i];
				mindigits[i] = nDig;
				maxdigits[i] = nDig;
				continue;
			}
			testdigits[i] = ((maxdigits[i] - mindigits[i]) / 2) + mindigits[i];
		}
	}
}

int main(int argc, char **argv)
{
	int ic, base, num_cases;

	if(scanf("%d %d", &(base), &(num_cases)) == 2){
		for(ic = 0; ic < num_cases; ic++){
			check_case(ic, base);
		}
	} else {
		fprintf(stderr, "Error reading base and number of cases\n");
	}
	return(0);
}
