#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#define MAX_PTS	50
#define MAX_NESTS	100
#define EPS (.0001)
#define OV_RT	1
#define OV_LFT	2

#undef DB_OUT

long geox[MAX_PTS], geoy[MAX_PTS], nestx[MAX_NESTS];
long peakx[MAX_PTS], peaky[MAX_PTS], maxx, maxy;

int npts, nnests, inmaxy, inmaxyloc;
double rainrate;

typedef struct _spline_seg_ {
	double B;
	double base;
	double C;
	long xl;
	long xr;
	long yl;
	long yr;
	long ymin;
} SPLINE_SEG,*PSPLINE_SEG;

SPLINE_SEG spsegs[2*MAX_PTS+2];
PSPLINE_SEG pLWallSeg, pRWallSeg;

typedef struct _peak_
{
	long x;
	long y;
	struct _peak_ *pLPar;
	struct _peak_ *pRPar;
	struct _peak_ *pLeft;
	struct _peak_ *pRt;
	PSPLINE_SEG pSegl;
	PSPLINE_SEG pSegr;
} PEAK, *PPEAK;

PEAK peaks[MAX_PTS];
int npeaks, maxpeak;
long maxpkx, maxpky;
PPEAK peakRoot, pLWall, pRWall;

typedef struct _gull_data_
{
	struct _gull_data_ *pNext;
	long x;
	double y;
	double baseVol;
	double baseTime;
	double remVol;
	double curTime;
	double floodTime;
} GULL, *PGULL;

GULL gulls[MAX_NESTS];
PGULL curGull;

typedef struct _valley_
{
	PGULL gullList;
	struct _valley_ *pOvFValley, *pPar, *pLChild, *pRChild;
	PSPLINE_SEG pLSeg, pRSeg;
	double xl;
	double yl;
	double xr;
	double yr;
	double vol;
	double xmin, ymin, ymax;
	double fillrate;
	double baseFillTime;
	double curTime;
	double curFill;
	int sortIndex;
	int overflowTo;
} VALLEY, *PVALLEY;

VALLEY valleys[2*MAX_PTS];
PVALLEY valleyList[2*MAX_PTS], pVRoot;
int nxtValley, nxtGull, sortCnt, remGulls;
double nxtGullY, nxtGullX;
double globTime;

double SplineX2Y(double x, PSPLINE_SEG ps)
{
	double ret;
	ret = ps->B*(x - ps->base) + ps->C;
	return ret;
}

int SplineY2XL(double y, double *pres, PSPLINE_SEG *pps)
{
	double x1, xc;
	PSPLINE_SEG ps, pst;
	ps = *pps;
	xc = *pres;
	if(ps == pLWallSeg) {
		pst = ps;
		pst++;
		if(y > (double)(pst->yl)) {
			*pres = (double)ps->xl;
			return 0;
		} else {
			ps++;
		}
	}
	if(ps->B > 0.0) {
		return -1;
	}
	while (y < (double)ps->ymin) {
		ps++;
		if(ps->B > 0.0) {
			return -1;
		}
	}
	*pps = ps;
	if(fabs(ps->B) < EPS) {
		if((double)ps->xl < xc) {
			x1 = xc;
		} else {
			x1 = (double)ps->xl;
		}
	} else {
		x1 = (y - ps->C)/(ps->B) + ps->base;
	}
	if((x1 < (double)ps->xl) || (x1 > (double)ps->xr)) {
		return -4;
	}
	*pres = x1;
	return 0;
}

int SplineY2XR(double y, double *pres, PSPLINE_SEG *pps)
{
	double x1, xc;
	PSPLINE_SEG ps, pst;
	ps =*pps;
	xc = *pres;
	if(ps == pRWallSeg) {
		pst = ps;
		pst--;
		if(y > (double)pst->yr) {
			*pres = (double)ps->xr;
			return 0;
		} else {
			ps--;
		}
	}
	if (ps->B < 0.0){
		return -1;
	}
	while (y < (double)ps->yl) {
		ps--;
		if(ps->B < 0.0) {
			return -1;
		}
	}
	*pps = ps;
	if(fabs(ps->B) < EPS) {
		if((double)ps->xl > xc) {
			return -17;
		}
		x1 = (double)ps->xl;
	} else {
		x1 = (y - ps->C)/(ps->B) + ps->base;
	}
	if((x1 < (double)ps->xl) || (x1 > (double)ps->xr)) {
		return -4;
	}
	*pres = x1;
	return 0;
}

double SplineArea(double xl, double xr, double ytop, PSPLINE_SEG ps)
{
	double yl, yr, ret;
	if(xl < (double)ps->xl) xl = (double)ps->xl;
	if(xr > (double)ps->xr) xr = (double)ps->xr;
	if(xr <= xl) return 0.0;
	yl = ps->B*(xl - ps->base) + ps->C;
	yr = ps->B*(xr - ps->base) + ps->C;
	ret = (ytop - 0.5*(yl + yr))*(xr - xl);
	return ret;
}

double ValleyArea(double xl, double xr, double ytop, PVALLEY pV)
{
	PSPLINE_SEG ps = pV->pLSeg;
	double ret = 0.0;
	while((ps != pRWallSeg) && (ps->xr <= xl)) {
		ps++;
	}
	if(ps == pRWallSeg) {
		fprintf(stderr, "Off the end in ValleyArea\n");
		return -1.0;
	}
	while((ps != pRWallSeg) && (ps->xr <= xr)) {
		ret += SplineArea(xl, ps->xr, ytop, ps);
		xl = ps->xr;
		ps++;
	}
	ret += SplineArea(xl, xr, ytop, ps);
	return ret;
}

int InitValley(PVALLEY pV, PPEAK pLeft, PPEAK pRt, PVALLEY pParent)
{
	PSPLINE_SEG ps;
	double vol, x;
	int ret, needleft, needrt;
	needleft = needrt = 0;
	pV->gullList = NULL;
	pV->pPar = pParent;
	pV->yl = pLeft->y;
	pV->yr = pRt->y;
	pV->curFill = 0.0;
	pV->curTime = 0.0;
	pV->fillrate = rainrate * (pRt->x - pLeft->x);
	if(pV->fillrate < EPS) {
		fprintf(stderr, "b add fill rate %f in InitValley\n", pV->fillrate);
		return -6;
	}
	if(pLeft == pLWall) {
		x = pV->xl = pLeft->x;
		pV->xr = pRt->x;
		pV->ymax = pRt->y;
		pV->pRSeg = pRt->pSegl;
		ps = pV->pLSeg = pLeft->pSegr;
		ps++;
		if(ps->yl > pV->ymax) needleft = 1; // left end not in pV
		pV->overflowTo = OV_RT;
	} else if(pRt == pRWall) {
		pV->xl = pLeft->x;
		x = pV->xr = pRt->x;
		pV->ymax = pLeft->y;
		ps = pV->pRSeg = pRt->pSegl;
		ps--;
		if(ps->yr > pV->ymax) needrt = 1; // rt end not in pV
		pV->pLSeg = pLeft->pSegr;
		pV->overflowTo = OV_LFT;
	} else if(pLeft ->y <= pRt->y) {
		pV->xl = pLeft->x;
		pV->ymax = pLeft->y;
		pV->pLSeg = pLeft->pSegr;
		pV->overflowTo = OV_LFT;
		ps = pRt->pSegl;
		x = pRt->x;
		needrt = 1;
	} else {
		pV->xr = pRt->x;
		pV->ymax = pRt->y;
		pV->pRSeg = pRt->pSegl;
		pV->overflowTo = OV_RT;
		ps = pLeft->pSegr;
		x = pLeft->x;
		needleft = 1;
	}
	if(needrt) {
		ret = SplineY2XR(pV->ymax, &x, &ps);
		if(ret < 0) {
			fprintf(stderr, "find rt x ret %d\n", ret);
			return -1;
		}
		pV->xr = x;
		pV->pRSeg = ps;
	}
	if(needleft) {
		ret = SplineY2XL(pV->ymax, &x, &ps);
		if(ret < 0) {
			fprintf(stderr, "find left x ret %d\n", ret);
			return -2;
		}
		pV->xl = x;
		pV->pLSeg = ps;
	}
	vol = ValleyArea(pV->xl, pV->xr, pV->ymax, pV);
	if(vol < 0.0) {
		fprintf(stderr, "Bad Vol in InitValley %f\n", vol);
		return -5;
	}
	pV->vol = vol;
	pV->baseFillTime = pV->vol/pV->fillrate;
	return 0;
}

int FindValleyMin(PVALLEY pV)
{
	PSPLINE_SEG pSeg;
	double xl, xr, y;
	if(pV->pLSeg == pLWallSeg) {
		pSeg = pV->pLSeg;
		xl = pV->xl;
		pSeg++;
		if(pSeg->B > 0.0) {
			pV->xmin = geox[0];
			pV->ymin = geoy[0];
			return 0;
		}
	}
	if(pV->pRSeg == pRWallSeg) {
		pSeg = pV->pRSeg;
		xl = pV->xr;
		pSeg--;
		if(pSeg->B < 0.0) {
			pV->xmin = geox[npts-1];
			pV->ymin = geoy[npts-1];
			return 0;
		}
	}
	pSeg = pV->pLSeg;
	if(pSeg == pLWallSeg) pSeg++;
	while (pSeg->B < 0) {
		if(pSeg == pV->pRSeg) {
			fprintf(stderr, "out of segs in FindValleyMin\n");
			return -11;
		}
		pSeg++;
	}
	xl = pSeg->xl; y = pSeg->yl;
	while (fabs(pSeg->B) < 0.0){
		if(pSeg == pV->pRSeg) {
			pV->xmin = xl;
			pV->ymin = y;
			return 0;
		}
		pSeg++;
	}
	xr = pSeg->xl;
	pV->xmin = 0.5*(xl + xr);
	pV->ymin = y;
	return 0;
}

int SetValleys(PVALLEY pV, PVALLEY pVPar, PPEAK pPkL, PPEAK pPkR)
{
	int ret;
	PPEAK pPk;
	PVALLEY pVL, pVR;
	if((ret = InitValley(pV, pPkL, pPkR, pVPar)) != 0) {
		return ret;
	}
	if(pPkL->y < pPkR->y) {
		pPk = pPkL->pRt;
	} else {
		pPk = pPkR->pLeft;
	}
	if(pPk != NULL) {
		pVL = &(valleys[nxtValley++]); 
		pVR = &(valleys[nxtValley++]); 
		pV->ymin = pPk->y;
		pV->pLChild = pVL;
		pV->pRChild = pVR;
		pVL->pOvFValley = pVR;
		pVR->pOvFValley = pVL;
		if((ret = SetValleys(pVL, pV, pPkL, pPk)) != 0) {
			return ret;
		}
		if((ret = SetValleys(pVR, pV, pPk, pPkR)) != 0) {
			return ret;
		}
		pV->xmin = 0.5*(pVL->xl + pVR->xr);
	} else {
		pV->pLChild = NULL;
		pV->pRChild = NULL;
		if((ret = FindValleyMin(pV)) != 0) {
			return ret;
		}
	}
	return 0;
}

int SetGulls(PVALLEY pV)
{
	PSPLINE_SEG ps;
	double xc, vol;
	int ret;
	if(nxtGull > nnests) {
		return 0;
	}
	if(nxtGullX < pV->xl) {
		fprintf(stderr, " nxt gull X %f < valley left %f\n", nxtGullX, pV->xl);
		return -21;
	}
	if(pV->pLChild == NULL) {
		while((nxtGull <= nnests) && (nxtGullX <= pV->xr)) {
			if(nxtGullX < pV->xmin) {
				//find rt x
				ps = pV->pRSeg;
				xc = ps->xr;
				if((ret = SplineY2XR(nxtGullY, &xc, &ps)) != 0) {
					fprintf(stderr, "find rt x failed for gullat %f\n", nxtGullX);
					return ret;
				}
				if((vol = ValleyArea(nxtGullX, xc, nxtGullY, pV)) < 0.0) {
					fprintf(stderr, "find gull vol failed for gullat %f\n", nxtGullX);
					return -6;
				}
				curGull->baseVol = curGull->remVol = vol;
				curGull->baseTime = vol/pV->fillrate;
			} else {
				// find left x
				ps = pV->pLSeg;
				xc = ps->xl;
				if((ret = SplineY2XL(nxtGullY, &xc, &ps)) != 0) {
					fprintf(stderr, "find left x failed for gullat %f\n", nxtGullX);
					return ret;
				}
				if((vol = ValleyArea(xc, nxtGullX, nxtGullY, pV)) < 0.0) {
					fprintf(stderr, "find gull vol failed for gullat %f\n", nxtGullX);
					return -6;
				}
				curGull->baseVol = curGull->remVol = vol;
				curGull->baseTime = vol/pV->fillrate;
			}
			curGull->pNext = pV->gullList;
			pV->gullList = curGull;
			curGull++;
			nxtGull++;
			nxtGullX = curGull->x;
			nxtGullY = curGull->y;
		}
	} else {
		while((nxtGull <= nnests) && (nxtGullY >= pV->ymin) && (nxtGullX <= pV->xmin)) {
			ps = pV->pRSeg;
			xc = ps->xr;
			if((ret = SplineY2XR(nxtGullY, &xc, &ps)) != 0) {
				fprintf(stderr, "find rt x failed for gullat %f\n", nxtGullX);
				return ret;
			}
			if((vol = ValleyArea(nxtGullX, xc, nxtGullY, pV)) < 0.0) {
				fprintf(stderr, "find gull vol failed for gullat %f\n", nxtGullX);
				return -6;
			}
			curGull->pNext = pV->gullList;
			pV->gullList = curGull;
			curGull->baseVol = curGull->remVol = vol;
			curGull->baseTime = vol/pV->fillrate;
			curGull++;
			nxtGull++;
			nxtGullX = curGull->x;
			nxtGullY = curGull->y;
		}
		if((ret = SetGulls(pV->pLChild)) != 0) {
			return ret;
		}
		if((ret= SetGulls(pV->pRChild)) != 0) {
			return ret;
		}
		while((nxtGull <= nnests) && (nxtGullX <= pV->xr)) {
			ps = pV->pLSeg;
			xc = ps->xl;
			if((ret = SplineY2XL(nxtGullY, &xc, &ps)) != 0) {
				fprintf(stderr, "find left x failed for gullat %f\n", nxtGullX);
				return ret;
			}
			if((vol = ValleyArea(xc, nxtGullX, nxtGullY, pV)) < 0.0) {
				fprintf(stderr, "find gull vol failed for gullat %f\n", nxtGullX);
				return -6;
			}
			curGull->pNext = pV->gullList;
			pV->gullList = curGull;
			curGull->baseVol = curGull->remVol = vol;
			curGull->baseTime = vol/pV->fillrate;
			curGull++;
			nxtGull++;
			nxtGullX = curGull->x;
			nxtGullY = curGull->y;
		}
	}
			
	return 0;
}

int VSrtFun(const void *p1, const void *p2)
{
	double t1, t2;
	PVALLEY pv1, pv2;
	pv1 = *(PVALLEY *)p1;
	pv2 = *(PVALLEY *)p2;
	t1 = pv1->baseFillTime;
	t2 = pv2->baseFillTime;
	if(t1 < t2) {
		return 1;
	} else {
		return -1;
	}
}

int SortValleys()
{
	int i;
	for(i = 0; i < nxtValley ; i++) {
		valleyList[i] = &(valleys[i]);
	}
	sortCnt = nxtValley;
	qsort(&(valleyList[0]), sortCnt, sizeof(PVALLEY), VSrtFun);
	for(i = 0; i < sortCnt ; i++) {
		valleyList[i]->sortIndex = i;
	}
	remGulls = nnests;
	return 0;
}

void dumpValleys()
{
	PVALLEY pV, pVbase;
	int i;
	pV = pVbase = &(valleys[0]);
	for(i = 0; i < nxtValley; i++, pV++) {
		printf("%d X %0.3f %0.3f Y %0.3f %0.3f V %0.3f Mn %0.3f %0.3f FT %0.3f LC %ld RC %ld\n",
			i, pV->xl, pV->xr, pV->yl, pV->yr, pV->vol, pV->xmin, pV->ymin, pV->baseFillTime, 
			pV->pLChild - pVbase, pV->pRChild - pVbase);
	}
}

int SetValleysGulls()
{
	PVALLEY pVL, pVR;
	int ret;
	double vol;
	curGull = &(gulls[0]);
	nxtGull = 1;
	nxtGullY = curGull->y;
	nxtGullX = curGull->x;
	pVRoot = &(valleys[0]);
	pVRoot->gullList = NULL;
	pVRoot->pPar = NULL;
	pVRoot->yl = geoy[0];
	pVRoot->yr = geoy[npts-1];
	pVRoot->curFill = 0.0;
	pVRoot->curTime = 0.0;
	pVRoot->fillrate = rainrate * (geox[npts-1] - geox[0]);
	if(pVRoot->fillrate < EPS) {
		fprintf(stderr, "b add fill rate %f in InitValley\n", pVRoot->fillrate);
		return -6;
	}
	pVRoot->xl = geox[0];
	pVRoot->xr = geox[npts-1];
	pVRoot->pRSeg = pRWall->pSegl;
	pVRoot->pLSeg = pLWall->pSegr;
	pVRoot->overflowTo = OV_RT;
	if(peakRoot == NULL) {
		nxtValley = 1;
		pVRoot->pLChild = NULL;
		pVRoot->pRChild = NULL;
		pVRoot->ymax = pVRoot->yl;
		if(pVRoot->ymax < pVRoot->yr) pVRoot->ymax = pVRoot->yr;
	} else {
		pVL = &(valleys[1]);
		pVR = &(valleys[2]);
		nxtValley = 3;
		pVRoot->pLChild = pVL;
		pVRoot->pRChild = pVR;
		pVL->pOvFValley = pVR;
		pVR->pOvFValley = pVL;
		pVRoot->ymax = peakRoot->y;
		if(pVRoot->ymax < pVRoot->yl) pVRoot->ymax = pVRoot->yl;
		if(pVRoot->ymax < pVRoot->yr) pVRoot->ymax = pVRoot->yr;
	}
	vol = ValleyArea(pVRoot->xl, pVRoot->xr, pVRoot->ymax, pVRoot);
	if(vol < 0.0) {
		fprintf(stderr, "Bad Vol for pVRoot %f\n", vol);
		return -5;
	}
	pVRoot->vol = vol;
	pVRoot->baseFillTime = pVRoot->vol/pVRoot->fillrate;
	if(peakRoot == NULL) {
		if((ret = FindValleyMin(pVRoot)) != 0) {
			return ret;
		}
	} else {
		pVRoot->ymin = peakRoot->y;
		pVRoot->xmin = peakRoot->x;
		if((ret = SetValleys(pVL, pVRoot, pLWall, peakRoot)) != 0) {
			return ret;
		}
		if((ret = SetValleys(pVR, pVRoot, peakRoot, pRWall)) != 0) {
			return ret;
		}
	}
#ifdef DB_OUT
	dumpValleys();
#endif
	if((ret = SetGulls(pVRoot)) != 0) {
		return ret;
	}
	return 0;
}

int SetGullHeights()
{
	int i, j;
	double y;
	PGULL pg;
	PSPLINE_SEG ps;
	pg = &gulls[0];
	ps = &spsegs[0];
	for(i = 0, j = 0; i < nnests ; i++, pg++) {
		while((ps->xr < pg->x) && (j < npts)) {
			ps++;
			j++;
		}
		if(j >= npts) {
			fprintf(stderr, "of the end at gull %d in SetGullHeights()\n", i+1);
			return -1;
		}
		y = SplineX2Y((double)pg->x, ps);
		pg->y = y;
		pg->baseVol = 0.0;
		pg->curTime = 0.0;
		pg->remVol = 0.0;
		pg->floodTime = 0.0;
	}
	return 0;
}

void dumpGulls()
{
	int i;
	PGULL pg = &(gulls[0]);
	printf("\n");
	for(i = 0; i < nnests ; i++, pg++) {
		printf("%d, %ld %f V %f T %f\n", i, pg->x, pg->y, pg->baseVol, pg->baseTime);
	}
}

int SetPeaks()
{
	int i;
	PSPLINE_SEG ps, psnxt;
	PPEAK pp, pt, pl, proot, pprev;
	npeaks = 2;
	maxpky = -1000000000;
	proot = pprev = NULL;
	ps = pRWallSeg;
	pp = &(peaks[0]);
	pp->x = ps->xl;
	pp->y = ps->yl;
	pp->pSegl = pp->pSegr = ps;
	pRWall = pp;
	pp++;
	ps = &(spsegs[0]);
	pp->x = ps->xl;
	pp->y = ps->yl;
	pp->pSegl = pp->pSegr = ps;
	pLWall = pp;
	pp++; ps++;
	psnxt = ps;
	psnxt++;
	for(i = 1; i <= (npts-1); i++, ps++, psnxt++) {
		if((ps->B > 0.00) && (psnxt->B <0.0)) {
			pp->x = psnxt->xl; pp->y = psnxt->yl;
			pp->pSegl = ps; pp->pSegr = psnxt;
		} else {
			continue;
		}
		if(proot == NULL) {
			pp->pLPar = pLWall;
			pp->pRPar = pRWall;
			proot = pp;
			pp->pLeft = NULL;
			pp->pRt = NULL;
		} else {
			if(pp->y <= pprev->y) {
				pp->pLPar = pprev;
				pp->pRPar = pRWall;
				pp->pLeft = NULL;
				pp->pRt = NULL;
				pprev->pRt = pp;
			} else {
				pp->pRPar = pRWall;
				pp->pLeft = pprev;
				pp->pRt = NULL;
				pt = pprev;
				pl = pprev;
				while(pt->y <= pp->y) {
					pt->pRPar = pp;
					if(pt->y > pl->y) {
						pl = pt;
					}
					pt = pt->pLPar;
				}
				pp->pLeft = pl;
				pp->pLPar = pt;
				pt->pRt = pp;
			}
		}
		pprev = pp;
		if(pp->y > maxpky) {
			proot = pp;
			maxpky = pp->y;
			maxpkx = pp->x;
			maxpeak = npeaks;
		}
		pp++;
		npeaks++;
	}
	peakRoot = proot;
	return 0;
}

void dumpPeaks()
{
	int i;
	PPEAK pp, pbase;
	printf("\n");
	pp = pbase = &(peaks[0]);
	for(i = 0; i < npeaks; i++, pp++) {
		printf("%d xy %ld %ld LI %ld RI %ld LPI %ld RPI %ld\n",
			i, pp->x, pp->y, pp->pLeft - pbase, pp->pRt - pbase,
			pp->pLPar - pbase, pp->pRPar - pbase);
	}
}

long SetSplineSeg(long x1, long y1, long x2, long y2, PSPLINE_SEG ps)
{
	long dx;
	dx = x2 - x1;
	if(abs(dx) < 1) {
		return -20;
	}
	if(x1 < x2) {
		ps->xl = x1; ps->xr = x2; ps->yl = y1; ps->yr = y2;
	} else {
		ps->xl = x2; ps->xr = x1; ps->yl = y2; ps->yr = y1;
	}
	ps->C = (double)ps->yl;
	ps->base = (double)ps->xl;
	ps->B = ((double)(ps->yr - ps->yl))/((double)(ps->xr - ps->xl));
	if(ps->yl < ps->yr) {
		ps->ymin = ps->yl;
	} else {
		ps->ymin = ps->yr;
	}
	return 0;
}

int SetSplines()
{
	int i;
	PSPLINE_SEG ps;
	pLWallSeg = ps = &(spsegs[0]);
	ps->B = 0.0;
	ps->xl = ps->xr = geox[0]; ps->base = (double)geox[0];
	ps->C = 10.e9; ps->yl = ps->yr = ps->ymin = 1000000000;
	pRWallSeg = ps = &(spsegs[npts]);
	ps->B = 0.0;
	ps->xl = ps->xr = geox[npts-1]; ps->base = (double)geox[npts-1];
	ps->C = 1.0e9; ps->yl = ps->yr = ps->ymin = 1000000000;
	for(i = 0; i < (npts - 1); i++) {
		if(SetSplineSeg(geox[i], geoy[i], geox[i+1], geoy[i+1], &(spsegs[i+1])) != 0) {
			return -37;
		}
	}
	return 0;
}

void dumpSplines()
{
	int i;
	PSPLINE_SEG ps = &(spsegs[0]);
	for(i =0; i <= npts; i++, ps++) {
		printf("%d %ld %ld %ld %ld BCbase %0.3f %0.3f %0.3f ymin %ld\n",
			i, ps->xl, ps->xr, ps->yl, ps->yr, ps->B, ps->C, ps->base, ps->ymin);
	}
}

char *ScanInt(char *pb, int *pval)
{
	char c;
	int val, stat, sign;
	val = stat = 0; sign = 1;
	while((c = *pb++) != 0){
		if(c == '-') {
			if(stat == 1) {
				*pval = -1000000;
				return NULL;
			}
			sign = -1;
			stat = 1;
		}
		else if((c >= '0') && ( c <= '9')) {
			val = val*10 +  (c - '0');
			stat = 1;
		} else if(stat == 1){
			*pval = sign*val;
			return pb;
		}
	}
	if(stat == 1) {
		*pval = sign*val;
		return pb;
	}
	*pval = -1000001;
	return NULL;
}

int ParseNests(char *pbuf)
{
	int x, i;
	char *pb = pbuf;
	for(i = 0; i < nnests ; i++) {
		if((pb = ScanInt(pb, &x)) == NULL) {
			fprintf(stderr, "ParseError on nest x %d\n", i+1);
			return -23;
		}
		nestx[i] = gulls[i].x = x;
		gulls[i].pNext = NULL;
	}
	return 0;
}

int ParseGeoPts(char *pbuf)
{
	int x, y, i, prevy;
	char *pb = pbuf;
	inmaxy= -1000000; inmaxyloc = -1; prevy = 1000000000;
	for(i = 0; i < npts ; i++) {
		if((pb = ScanInt(pb, &x)) == NULL) {
			fprintf(stderr, "ParseError on geo x %d\n", i+1);
			return -24;
		}
		if((pb = ScanInt(pb, &y)) == NULL) {
			fprintf(stderr, "ParseError on geo y %d\n", i+1);
			return -25;
		}
		if(y == prevy) {
			fprintf(stderr, "adjacent equal y values %d at %d, %d\n", y, i, i+1);
			return -26;
		}
		if(y > inmaxy) {
			inmaxy = y;
			inmaxyloc = i;
		}
		geox[i] = x;
		geoy[i] = y;
		prevy = y;
	}
	return 0;
}

int flag = 0;

int ProcOverflow(PVALLEY pV, PVALLEY pOV, int type, double addRate)
{
	PGULL pG, pPrev;
	PVALLEY pTV;
	int i;
	if(pOV->curFill >= pOV->vol) {	//if ocerflow valley is full try its overflow
		pTV = pOV->pOvFValley;
		if(pTV->curFill >= pTV->vol) { // if other half full (already processed) return
			return 0;
		} else {	// work on complement
			pOV = pTV;
		}
	}
	//now pOV is a not full valley getting overflow from pV
	pOV->curFill += pOV->fillrate*(globTime - pOV->curTime);
	pPrev = NULL;
	pG = pOV->gullList;
	while(pG != NULL) {		//scan gulls
		if(pG->baseVol <= pOV->curFill) { // if gull now flooded figure out when
			pG->floodTime = pG->curTime + pG->remVol/pOV->fillrate;
			remGulls--;	// and remove it from the list
			if(pPrev == NULL) {
				pOV->gullList = pG->pNext;
			} else {
				pPrev->pNext = pG->pNext;
			}
			pG = pG->pNext;
		} else {	// get current fill (remvol) and time wait for next time
			pG->remVol -= pOV->fillrate*(globTime - pG->curTime);
			pG->curTime = globTime;
			pPrev = pG;
			pG = pG->pNext;
		}
	}
	pOV->curTime = globTime;
	pOV->fillrate += addRate;	// add rate from pV
	pOV->baseFillTime = globTime + (pOV->vol - pOV->curFill)/pOV->fillrate;
	// move pOV up in list if necessary
	for(i = pOV->sortIndex + 1; i < sortCnt ; i++) {
		if(valleyList[i]->baseFillTime > pOV->baseFillTime) {
			valleyList[i-1] = valleyList[i];
			valleyList[i-1]->sortIndex = i-1;
		} else break;
	}
	valleyList[i-1] = pOV;
	pOV->sortIndex = i-1;
	if(pOV->pLChild != NULL) { // pOV not full, children also get overflow from pV
		if(type == OV_RT) {
			return ProcOverflow(pOV, pOV->pLChild, type, addRate);
		} else {
			return ProcOverflow(pOV, pOV->pRChild, type, addRate);
		}
	}
	return 0;
}

int ProcNxtValley()
{
	PVALLEY pV, pOvV;
	PGULL pG;
	sortCnt--;
	pV = valleyList[sortCnt];
	pG = pV->gullList;
	globTime = pV->baseFillTime;
	while (pG != NULL) {
		pG->floodTime = pG->curTime + pG->remVol/pV->fillrate;
		remGulls--;
		pG = pG->pNext;
	}
	pV->gullList = NULL;
	pV->curFill = pV->vol;
	if(pV ==  &(valleys[2])) {
		flag++;
	}
	pOvV = pV->pOvFValley;
	if(pOvV == NULL) {
		return 0;
	}
	return ProcOverflow(pV, pOvV, pV->overflowTo, pV->fillrate);
	return 0;
}

char inbuf[1024];
int main()
{
	int ret, i;
	PGULL pG = &(gulls[0]);
	if(fgets(&(inbuf[0]), 1024, stdin) == NULL) {
		fprintf(stderr, "read failed on point count and rain rate\n");
		return -1;
	}
	if(sscanf(&(inbuf[0]), "%d %lf %d", &npts, &rainrate, &nnests) != 3) {
		fprintf(stderr, "scan failed on point count, rain rate and next cnt\n");
		return -2;
	}
	if((npts < 1) || (npts > MAX_PTS)) {
		fprintf(stderr, "point count %d not in range 1 ..%d\n", npts, MAX_PTS);
		return -3;
	}
	inmaxy= -1000000; inmaxyloc = -1;
	if(fgets(&(inbuf[0]), 1024, stdin) == NULL) {
		fprintf(stderr, "read failed on geo points\n");
		return -1;
	}
	if((ret = ParseGeoPts(&(inbuf[0]))) != 0) {
		return ret;
	}
	if(fgets(&(inbuf[0]), 1024, stdin) == NULL) {
		fprintf(stderr, "read failed on nest locations\n");
		return -1;
	}
	if((ret = ParseNests(&(inbuf[0]))) != 0) {
		return ret;
	}
	if((ret = SetSplines()) != 0) {
		return ret;
	}
#ifdef DB_OUT
	dumpSplines();
#endif
	if((ret = SetPeaks()) != 0) {
		return ret;
	}
#ifdef DB_OUT
	dumpPeaks();
#endif
	if((ret = SetGullHeights()) != 0) {
	
		return ret;
	}
#ifdef DB_OUT
	dumpGulls();
#endif
	if((ret = SetValleysGulls()) != 0) {
		return ret;
	}
#ifdef DB_OUT
	dumpGulls();
#endif
	SortValleys();
	globTime = 0.0;
	while((remGulls > 0) && (sortCnt > 0)) {
		if((ret = ProcNxtValley()) != 0) {
			fprintf(stderr, "procvalley ret %d gull %d sort %d\n", ret, remGulls, sortCnt);
			return ret;
		}
	}
	for(i = 0; i < nnests ; i++, pG++){
		printf("%0.8lf\n", pG->floodTime);
	}
	return 0;
}

