#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

typedef unsigned char BYTE;
#define PQMAX	10100
#undef DEBUG

int flag = 0;
typedef struct _cont_link_
{
	struct _cont_link_ *pNext;
	BYTE a, b, c;
} CONT_LINK, *PCONT_LINK;

typedef struct _seg_
{
	PCONT_LINK pconts;
	int len;
} SEG, *PSEG;

typedef struct _snode_
{
	struct _snode_ *pNext;
	struct _snode_ *pPred;
	int cur;
	int prev;
	int len;
	int con_len;
}SNODE, *PSNODE;

PSNODE freeSN, ptNode;

typedef struct _inter_
{
	int dist;
	int pred;
	PSNODE pLinks;
} INTER, *PINTER;

int ninter, nseg, ncpairs, maxdist, start, end;

SEG segs[101][101];
INTER inters[101];
CONT_LINK conlinks[101*100];
SNODE snodes[10100];
PSNODE PQ[10101];
int PQlen[10100];

int PQsz, nxtnode;

PSNODE GetNode()
{
	PSNODE ret;
	if(freeSN != NULL) {
		ret = freeSN;
		freeSN = ret->pNext;
		return ret;
	}
	if(nxtnode >= 10100) {
		return NULL;
	}
	ret = &snodes[nxtnode];
	nxtnode++;
	return ret;
}

void ReleaseNode(PSNODE p)
{
	p->pNext = freeSN;
	freeSN = p;
}

void PQSiftUp()
{
	int i, j;
	PSNODE p = PQ[PQsz];
	i = PQsz; j = i/2;
	while (j >= 1) {
		if(p->len > PQ[j]->len) {
			PQ[i] = p;
			PQlen[i] = p->len;
			return;
		} else {
			PQ[i] = PQ[j];
			PQlen[i] = PQlen[j];
			i = j; j = i/2;
		}
	}
	PQ[1] = p;
	PQlen[i] = p->len;
}

PSNODE PQGetNxt()
{
	int i, j;
	PSNODE ret, p = PQ[PQsz];
	if(PQsz <= 0) {
		return NULL;
	}
	ret = PQ[1];
	PQsz--;
	i = 1; j= 2*i;
	while(j <= PQsz) {
		if((j <PQsz) && (PQ[j]->len > PQ[j+1]->len)) j++;
		if(p->len < PQ[j]->len) {
			PQ[i] = p;
			PQlen[i] = p->len;
			return ret;
		}
		PQ[i] = PQ[j];
		PQlen[i] = PQlen[j];
		i = j; j = 2*i;
	}
	PQ[i] = p;
	return ret;
}

int PQAdd(PSNODE p)
{
	if(PQsz >= PQMAX) {
		return -1;
	}
	PQsz++;
	PQ[PQsz] = p;
	PQlen[PQsz] = p->len;
	PQSiftUp();
	return 0;
}

void Init()
{
	int i, j;
	for(i = 0; i <= ninter ; i++) {
		inters[i].dist = 1000000000;
		inters[i].pLinks = NULL;
		inters[i].pred = 0;
		for(j = 0; j <= ninter;  j++) {
			segs[i][j].pconts = NULL;
			segs[i][j].len = 0;
		}
	}
	nxtnode = 0;
	PQsz = 0;
	freeSN = ptNode = NULL;
}

void Backtrack()
{
	PSNODE p = ptNode;
	while(p != NULL) {
		printf("%d ", p->cur);
		p = p->pPred;
	}
	printf("\n");
}

char inbuf[256];
int main()
{
	int i, a, b, c, notdone, tdist, len, clen, nlen, skip;
	int qlen, qclen, qcur, qprev;
	PSNODE p, q, qp, qr;
	PCONT_LINK pc, pconts = NULL;
	if(fgets(&(inbuf[0]), 255, stdin) == NULL) {
		fprintf(stderr, "read failed on parameters\n");
		return -1;
	}
	if(sscanf(&(inbuf[0]), "%d %d %d %d %d %d", 
		&ninter, &nseg, &ncpairs, &maxdist, &start, &end) != 6) {
		fprintf(stderr, "scan failed on parameters\n");
		return -2;
	}
	if((ninter < 2) || (ninter > 100)) {
		fprintf(stderr, "ninter %d not in range 2..100\n", ninter);
		return -3;
	}
	if((maxdist < 1) || (maxdist > 100)) {
		fprintf(stderr, "maxdist %d not in range 1..100\n", maxdist);
		return -4;
	}
	if((ncpairs < 0) || (ncpairs > (ninter*(ninter -1)))) {
		fprintf(stderr, "ncpairs %d not in range 0..%d\n", ncpairs, ninter*(ninter -1));
		return -5;
	}
	if((start < 1) || (start >ninter)) {
		fprintf(stderr, "start %d not in range 1..%d\n", start, ninter);
		return -6;
	}
	if((end < 1) || (end >ninter) || (end== start)) {
		fprintf(stderr, "end %d not in range 1..%d or end == start\n", end, ninter);
		return -7;
	}
	for(i = 1; i <= nseg; i++) {
		if(fgets(&(inbuf[0]), 255, stdin) == NULL) {
			fprintf(stderr, "read failed inter %d\n", i);
			return -11;
		}
		if(sscanf(&(inbuf[0]), "%d %d %d", &a, &b, &c) != 3) {
			fprintf(stderr, "scan failed oninter %d\n", i);
			return -12;
		}
		if((a < 1) || (a >ninter)) {
			fprintf(stderr, "pta %d not in range 1..%d segment %d\n",
				a, ninter, i);
			return -13;
		}
		if((b < 1) || (b >ninter) || (b== a)) {
			fprintf(stderr, "ptb %d not in range 1..%d or ptb == pta %d segment %d\n",
				b, ninter, a, i);
			return -14;
		}
		if((c < 1) || (c > 100)) {
			fprintf(stderr, "len %d not in range 1..100 segment %d\n", c, i);
			return -15;
		}
		segs[a][b].len = c;
		segs[b][a].len = c;
	}
	for(i = 1; i <= ncpairs; i++) {
		if(fgets(&(inbuf[0]), 255, stdin) == NULL) {
			fprintf(stderr, "read failed inter %d\n", i);
			return -21;
		}
		if(sscanf(&(inbuf[0]), "%d %d %d", &a, &b, &c) != 3) {
			fprintf(stderr, "scan failed oninter %d\n", i);
			return -22;
		}
		if((a < 1) || (a >ninter)) {
			fprintf(stderr, "pta %d not in range 1..%d pair %d\n",
				a, ninter, i);
			return -23;
		}
		if((b < 1) || (b >ninter) || (b== a)) {
			fprintf(stderr, "ptb %d not in range 1..%d or ptb == pta pair %d\n",
				b, ninter, i);
			return -24;
		}
		if((c < 1) || (c > ninter) || (c == a) || (c == b)) {
			fprintf(stderr, "ptc %d not in range 1..%d or ptc == pta or ptc == ptb pair %d\n",
				c, ninter, i);
			return -25;
		}
		if(segs[a][b].len == 0) {
			fprintf(stderr, "pair %d; %d, %d is not a segment\n", i, a, b);
			return -26;
		}
		if(segs[b][c].len == 0) {
			fprintf(stderr, "pair %d; %d, %d is not a segment\n", i, b, c);
			return -27;
		}
		conlinks[i].a = a;
		conlinks[i].b = b;
		conlinks[i].c = c;
		conlinks[i].pNext = segs[a][b].pconts;
		segs[a][b].pconts = &(conlinks[i]);
	}
	notdone = 1; tdist = 1000000000;
	p = GetNode();
	p->len = 0; p->con_len = 0; p->cur = start; p->prev = 0; p->pPred = NULL;
	PQAdd(p);
	inters[start].dist = 0;
	while(notdone) {
 		if((p = PQGetNxt()) == NULL) {
#ifdef DEBUG
			printf("Q empty\n");
#endif
			if(tdist < 1000000000) {
				printf("%d\n", tdist);
			} else {
				printf("impossible\n");
			}
			return 0;
		}
#ifdef DEBUG
		printf("proc n %d p %d l %d c %d\n", p->cur, p->prev, p->len, p->con_len);
#endif
		a = p->prev;
		b = p->cur;
		len = p->len; clen = p->con_len;
		if(len > tdist) {
			notdone = 0;
		}
		if(a > 0) pconts = segs[a][b].pconts;
		for( c = 1; c <= ninter; c++) {
			if((c == a) || (c == b)) continue;
			if((nlen = segs[b][c].len) > 0) {
				pc = pconts;
				skip = 0;
				while(pc != NULL) {
					if(pc->c == c) {
						skip = 1;
						break;
					}
					pc = pc->pNext;
				}
				if(skip && ((clen + nlen) > maxdist)) {
#ifdef DEBUG
					printf("cont %d: %d + %d > %d\n", c, clen, nlen, maxdist);
#endif
					continue;
				}
				// got to here make new node;
				q = GetNode();
				q->cur = qcur = c; q->prev = qprev = b; 
				q->len = qlen = len + nlen; q->pNext = NULL;
				q->pPred = p;
				if(skip)
					qclen = clen + nlen;
				else if(segs[b][c].pconts != NULL) {
					qclen = nlen;
				} else {
					qclen = 0;
				}
#ifdef DEBUG
				printf("\tadd n %d p %d l %d c %d ", qcur, qprev, qlen, qclen);
#endif
				q->con_len = qclen;
				if(inters[c].pLinks == NULL) {
					inters[c].pLinks = q;
					inters[c].dist = qlen;
					inters[c].pred = qprev;
					if(c == end) {
						tdist = qlen;
						ptNode = q;
					}
#ifdef DEBUG
					printf("add only\n");
#endif
					if(PQAdd(q) != 0) {
#ifdef DEBUG
						printf("Qfull\n");
#endif
						return -67;
					}
					continue;
				} else {
					q->pNext = inters[c].pLinks;
					inters[c].pLinks = q;
					qp = q; qr = qp->pNext; 
					while(qr != NULL) {
						if((qr->len <= qlen) && (qr->prev == b) &&
							(qr->con_len <= qclen)){
							// better tthan q
							inters[c].pLinks = q->pNext;
							ReleaseNode(q);
#ifdef DEBUG
							printf("too big\n");
#endif
							q = NULL;
							break;
						} else if((qr->len > qlen) && (qr->con_len > qclen)){
							qp->pNext = qr->pNext;
							ReleaseNode(qr);
							qr = qp->pNext;
						} else {
							qp = qr; qr = qp->pNext;
						}
					}
					if((q != NULL) && (qlen < inters[c].dist)) {
						inters[c].dist = qlen;
						inters[c].pred = qprev;
						if(c == end) {
							tdist = qlen;
							ptNode = q;
						}
					}
					if(q != NULL) {
						if(PQAdd(q) != 0) {
#ifdef DEBUG
							printf("Qfull\n");
#endif
							return -66;
						}
#ifdef DEBUG
						printf("added\n");
#endif
					}
				}
			}
		}
	}
	if(tdist < 1000000000) {
		printf("%d\n", tdist);
	} else {
		printf("impossible\n");
	}
#ifdef DEBUG
	Backtrack();
#endif
	return 0;
}

