/*
 * Morse Code Palindrom - shortest added string version
 *
 * Fred Pickel, Greater New York Judge
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

//#define DEBUG	// print all triples found, labelling duplicates
//#define DEBUG1	// print a message if no element of a triple is the input value

typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;

char inmorse[480];

// morse codes for chars (and digits below) '0' = dot, '1' = dash
char* charLookup[26] =
{
	"01", "1000", "1010", "100", "0", "0010", "110", "0000", "00",
	"0111", "101", "0100", "11", "10", "111", "0110", "1101", "010",
	"000", "1", "001", "0001", "011", "1001", "1011", "1100"
};

char * digLookup[10] =
{
	"11111", "01111", "00111", "00011", "00001",
	"00000", "10000", "11000", "11100", "11110"
};

// search tree to find longest dot-dash sequence belongiing to a char or digit
char trie[41][3] =
{
	{1, 2, 0},
	{3, 4, 'E'},
	{5, 6, 'T'},
	{7, 8, 'I'},	//3 .?
	{9, 10, 'A'},
	{11, 12, 'N'},	//5 -?
	{13, 14, 'M'},
	{15, 16, 'S'},	//7 ..?
	{17, 18, 'U'},
	{19, 0, 'R'},	//9 .-?
	{21, 22, 'W'},
	{23, 24, 'D'},	//11 -.
	{25, 26, 'K'},
	{27, 28, 'G'},	//13 --
	{29, 30, 'O'},
	{31, 32, 'H'},	//15 ...?
	{0, 33, 'V'},
	{0, 0, 'F'},	//17 ..-? 
	{0, 34, 0},
	{0, 0, 'L'},	//19 .-.?
	{0, 0, 0},		// not a char and no successors
	{0, 0, 'P'},	//21 .--?
	{0, 35, 'J'},
	{36, 0, 'B'},	//23 -..?
	{0, 0, 'X'},
	{0, 0, 'C'},	//25 -.-?
	{0, 0, 'Y'},
	{37, 0, 'Z'},	//27 --.
	{0, 0, 'Q'},
	{38, 0, 0},		//29 ---?
	{39, 40, 0},	
	{0, 0, '5'},	//31 ....?
	{0, 0, '4'},
	{0, 0, '3'},	//33 ...--?
	{0, 0, '2'},	//34 ..--= 
	{0, 0, '1'},	//35 .----
	{0, 0, '6'},	//36 -....?
	{0, 0, '7'},	//37 --...
	{0, 0, '8'},	//38 ---..
	{0, 0, '9'},	//39 ----. 
	{0, 0, '0'},	//40 -----
};

int ScanInput(char *pin, char *pout, int allowspace)
{
	char c, tc, *outp, *tmp;
	int outcnt, cnt;
	outcnt = cnt = 0;
	outp = pout;
	while(((c = *pin++) != 0) && (cnt <= 80)) {
		cnt++;
		if((c >= 'A') && ( c <= 'Z')) {
			tmp = charLookup[c - 'A'];
			while((tc = *tmp++) != 0) {
				*outp++ = tc;
				outcnt++;
			}
		} else if((c >= '0') && ( c <= '9')) {
			tmp = digLookup[c - '0'];
			while((tc = *tmp++) != 0) {
				*outp++ = tc;
				outcnt++;
			}
		} else if(!allowspace && (!isspace(c)) && (!ispunct(c))) {
			return -1;
		}
	}
	if(cnt > 80) {
		return -2;
	}
	*outp = 0;
	return outcnt;
}

/* 0 if *p of length len is a palindrome else -1 */
int IsPalin(char *p, int len)
{
	int i, lim;
	char *phd, *ptl;
	phd = p;
	ptl = p + (len -1);
	lim = len/2;
	for(i = 0; i < lim ; i++) {
		if(*phd != *ptl) {
			return -1;
		}
		phd++;
		ptl--;
	}
	return 0;
}

// return longest tail substring of p (length len) which is a palindrome
int FindTailPalin(char *p, int len)
{
	char scan[4], *ps, *pe, *pesav;
	int ret;
	if(len == 0) {
		return 0;
	} else if(len == 1) {
		return 1;
	} else if(len == 2) {
		if(p[0] == p[1]) {
			return 2;
		} else {
			return 1;
		}
	}
	pe = p + (len - 1);
	scan[0] = *pe--;
	scan[1] = *pe--;
	scan[2] = *pe--;
	scan[3] = 0;
	ret = len;
	pesav = pe;
	ps = p;
	while(ret >= 3) {
		pe = pesav;
		if(ps[0] == scan[0]) {
			if(ps[1] == scan[1]) {
				if(ps[2] == scan[2]) {
					if(IsPalin(ps, ret) == 0) {
						return ret;
					}
				}
			}
		}
		ps++;
		ret--;
	}
	if(pesav[0] == pesav[-1]) {
		return 2;
	}
	return 1;
}

int Is0139(char *p)
{
	if (p[0] == '0') {
		if(p[1] == '0') {
			if((p[2] == '0') && (p[3] == '1') && (p[4] == '1')) {
				return 3;
			} else {
				return -1;
			}
		} else if(p[1] == '1') {
			if((p[2] == '1') && (p[3] == '1') && (p[4] == '1')) {
				return 1;
			} else {
				return -1;
			}
		}
	} else if (p[0] == '1') {
		if((p[1] == '1') && (p[3] == '1') && (p[4] == '1')) {
			if(p[4] == '0') {
					return 9;
			} else if(p[4] == '1') {
				return 0;
			}
		}
	}
	return -1;
}

char GetNextChar(char *p, int rem, int *pd)
{
	int trieind, cnt, back, ind, lim;
	char res, prev, tmp;
	trieind = 0;
	cnt= 0;
	lim = 5;
	if(lim > rem) lim = rem;
	back = 0;
	ind = p[0] - '0';
	prev = res = 0;
	trieind = trie[0][ind];
	cnt = 1;
	while(cnt <= lim) {
		tmp = trie[trieind][2];
		if(tmp != 0) {
			prev = res;
			res = tmp;
			back = 0;
		} else {
			back++;
		}
		ind = p[cnt] - '0';
		cnt++;
		trieind = trie[trieind][ind];
		if(trieind == 0) {
			if((cnt == 5) && (back == 0) && (Is0139(p+3) >= 0)) {
				*pd = cnt - 2;
				return prev;
			} else {
				*pd = cnt - back - 1;
				return res;
			}
		}
	}
	*pd = cnt - back - 1;
	return res;
}

char initDigits[80];
char revMorse[480];

int FindAddedString(char *pinmorse, int inmorsecnt, char *resp)
{
	int initDigCnt;
	int palinCnt, rem, d, charcnt;
	char c, *p, *p1, *p2;
	palinCnt =  FindTailPalin(pinmorse, inmorsecnt);
	rem = inmorsecnt - palinCnt;	// what we have to match
	initDigCnt = 0;
	p = pinmorse;
#ifdef NOTDEF
	while((rem >= 5) && ((d = IsMorseDigit(p)) >= 0)) {
		crev = (char)((10 - d) % 10);	// reverse digit
		crev += '0';
		initDigits[initDigCnt] = crev;
		p += 5;
		rem -= 5;
		initDigCnt++;
	}
#endif
	// reverse stuff to do
	p1 = &(revMorse[0]); p2 = p + (rem-1);
	for(d = 0; d < rem ; d++) {
		*p1++ = *p2--;
	}
	*p1 = 0;
	p = &(revMorse[0]);
	charcnt = 0;
	while(rem > 0) {
		c = GetNextChar(p, rem, &d);
		resp[charcnt] = c;
		charcnt++;
		p += d;
		rem -= d;
	}
#ifdef NOTDET
	while(initDigCnt > 0) {
		initDigCnt--;
		c = initDigits[initDigCnt];
		resp[charcnt] = c;
		charcnt++;
	}
#endif
	resp[charcnt] = 0;
	return charcnt;
}

char valbuf[1024];

int Validate(char *pinmorse, int inmorselen, char *pres, int reslen)
{
	char *p;
	int resmorsecnt, totcnt;
	strncpy(&(valbuf[0]), pinmorse, inmorselen);
	p = &(valbuf[inmorselen]);
	if((resmorsecnt = ScanInput(pres, p, 0)) < 0) {
		printf("invalid res string\n");
		return -1;
	}
	totcnt = inmorselen + resmorsecnt;
	if(IsPalin(&(valbuf[0]), totcnt) != 0) {
		printf("not palindrome\n");
		return -2;
	}
	return 0;
}

char res[128];
char inbuf[256];
int main()
{
	int inmorsecnt, rescnt, ret;
	ret = 0;
	if(fgets(&(inbuf[0]), 255, stdin) == NULL) {
		fprintf(stderr, "read failed on input line\n");
		return -1;
	}
	if((inmorsecnt = ScanInput(&(inbuf[0]), &(inmorse[0]), 1)) < 0) {
		fprintf(stderr, "scan failed on input line\n");
		return -2;
	}
	if(IsPalin(&(inmorse[0]), inmorsecnt) == 0) {
		printf("0\n");
	} else {
		if((rescnt = FindAddedString(&(inmorse[0]), inmorsecnt, &(res[0]))) < 0){
			return -3;
		} else {
			if((ret = Validate(&(inmorse[0]), inmorsecnt, &(res[0]), rescnt)) == 0) {
				printf("%d %s\n", rescnt, &(res[0]));
			}
		}
	}
	return ret;
}

