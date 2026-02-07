#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

signed char code[26] = 
{-1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, 2, -1, 
 -1, -1, -1, -1, 3, -1, -1, 4, -1, -1, -1, -1, -1
};

char instr[40000], fliprev[40000];
int counts[5];

char inv[5] = {0, 4, 3, 2, 1};
int k, inlen, hlen, inr, inl, flipr, flipl, midlen, minrun, remlen;
char srchcd;
int rinr, rinl, rflipr, rflipl, srchind, runlen, isrun;

int ReadInt()
{
	char c;
	int r;
	c = getc(stdin);
	while(isspace(c)) {
		c = getc(stdin);
	}
	if(!isdigit(c)) {
		return -1;
	}
	r = c - '0';
	c = getc(stdin);
	while(isdigit(c)) {
		r = 10*r + (c - '0');
		c = getc(stdin);
	}
	return r;
}

int ReadString(int len)
{
	int i;
	char cd, c;
	for(i = 0; i < 5; i++) counts[i] = 0;
	c = getc(stdin);
	while(isspace(c)) {
		c = getc(stdin);
	}
	i = 0;
	while((i < len) && islower(c)) {
		if((cd = code[c - 'a']) < 0) {
			fprintf(stderr, " char %d: %c not valid\n", i+1, c);  
			return -3;
		}
		instr[i] = instr[i + len] = instr[i + 2*len] = instr[i + 3*len] = cd;
		counts[cd]++;
		i++;
		c = getc(stdin);
	}
	if(i < len) {
		fprintf(stderr, "num in chars %d < len %d\n", i, len);
		return -4;
	}
	return 0;
}

int CheckSides(int isrun)
{
	char icd, fcd;
	int i, j, k, m, n,l, remlen, len3, ok, count = 0;
	// scan in rt flip left
	icd = instr[inr+1]; fcd = fliprev[flipl-1];
	i = inr+1; j = flipl - 1;
	remlen = hlen - midlen;
	if(icd == fcd) {
		ok = 1;
		for(k = 0; k < remlen; k++, i++, j--) {
			if((instr[i] != icd) || (fliprev[j] != fcd)) {
				ok = 0;
				break;
			}
		}
		if(ok) {
			count = 4 + 6*(remlen - 1);
			if(isrun) {
				count += 6*(midlen - 1);
			}
			return count;
		}
	}
	i = inr+1; j = flipl - 1;
	for(k =1; k <= remlen ; k++, i++, j--) {
		if((instr[i] == fcd) && (fliprev[j] == icd)) {
			ok = 1;
			for(l = 0, m = inr + 2, n = j+1; l < k-2 ; l++, m++, n++) {
				if(instr[m] != fliprev[n]) {
					ok = 0;
					break;
				}
			}
			if(ok) {
				if(k == remlen) {
					count += 4;
					if(isrun) {
						count += 6*(midlen - 1);
					}
				} else {
					len3 = remlen - k;
					for(l = 0, m = inl - len3, n = flipr +1; l < len3; l++, m++, n++) {
						if(instr[m] != fliprev[n]) {
							ok = 0;
							break;
						}
					}
					if(ok) {
						count += 6;
					}
				}
			}
		}
	}
	return count;
}

int CheckRuns(int *pcount)
{
	char cd, ncd, icd, fcd;
	int i, j, k, len, maxlen, is, ok, remlen;
	maxlen = is = 0;
	for(i = inlen, len = 0, cd = 0; i < 2*inlen; i++){
		if((ncd = instr[i]) == cd) {
			len++;
		} else {
			if(len > maxlen) {
				maxlen = len;
				rinr = is + len -1;
				rinl = is;
				icd = cd;
			}
			len = 1;
			is = i;
			cd = ncd;
		}
	}
	if(len > maxlen) {
		maxlen = len;
		rinr = flipr =  is + len -1;
		rinl = flipl = is;
	}
	if(maxlen < minrun) {
		return 0;
	}
	for(i = inlen - maxlen + 1, len = 0; i < 2*inlen - maxlen; i++){
		if(fliprev[i] == icd) {
			if(len == 0) is = i;
			len++;
		} else {
			len = 0;
		}
		if(len >= maxlen) {
			rflipl = is;
			rflipr = is + maxlen - 1;
			break;
		}
	}
	if(len < maxlen) return 0;
	runlen = maxlen;
	icd = instr[rinr+1]; fcd = fliprev[rflipl-1];
	i = rinr+1; j = rflipl - 1;
	remlen = hlen - runlen;
	if(icd == fcd) {
		ok = 1;
		for(k =1; k <= remlen ; k++, i++, j--) {
			if((instr[i] != fcd) || (fliprev[j] != icd)) {
				ok = 0;
				break;
			}
		}
		if(ok) { // rectangle
			*pcount = 4 + 6*(runlen - 1) + 6*(remlen - 1);
			return 1;
		}
	}
	isrun = 1;
	srchind = rinr + 1;
	srchcd = instr[srchind];
	return 0;

}

int CheckMatch(int flipbase)
{
	int i, j;
	// check for long runs
	//see how wide the match is
	i = srchind; j = flipbase;
	while((i < srchind + hlen) && (instr[i] == fliprev[j])) {
		i++;
		j++;
	}
	inr = i- 1; flipr = j-1;
	i = srchind; j = flipbase;
	while((i > srchind - hlen) && (instr[i] == fliprev[j])){
		i--;
		j--;
	}
	inl = i+1; flipl = j+1;
	midlen = inr - inl + 1;
	if(midlen >= hlen) {
		return 0;
	}
	remlen = hlen - midlen;
	if(isrun && (remlen == runlen)) {
		if((rflipl == (flipr + 1)) && (rinr == (inl - 1))) {
			return (4 + 6*(runlen - 1));
		}
	}

	return CheckSides(0);
}

int main()
{
	int i, j, ret, count;
	if((inlen = ReadInt()) <= 0)  return (inlen - 1);
	if(inlen  > 10000) {
		fprintf(stderr, "inlen %d > 10000\n", inlen);
		return -1;
	}
	if(inlen  & 1) {
		fprintf(stderr, "inlen %d is odd\n", inlen);
		return -1;
	}
	hlen = inlen/2;
	minrun = inlen/10;
	if(minrun < 4) minrun = 4;
	if((ret = ReadString(inlen)) < 0) return ret;
	if((counts[1] != counts[4]) || (counts[2] != counts[3])) {
		fprintf(stderr, "dcnt %d ! = ucnt %d and/or lcnt %d != rcnt %d\n",
			counts[1], counts[4], counts[2], counts[3]);
	}
	for(i = 0, j = 4*inlen-1; i < 4*inlen ; i++, j--) {
		fliprev[j] = 5 -instr[i];
	}
	count = 0;
	srchind = inlen + hlen;
	srchcd = instr[srchind];
	if(CheckRuns(&count) == 1) {
		printf("%d\n", count);
		return 0;
	} else {
		for(i = inlen; i <2*inlen; i++) {
			if(srchcd == fliprev[i]) {
				count += CheckMatch(i);
			}
		}
	}
	printf("%d\n", count);
	return 0;
}

