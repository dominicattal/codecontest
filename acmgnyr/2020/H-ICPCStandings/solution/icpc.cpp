#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>

/*
 * Define DEBUG to see all input and sorting.
 * Define DEBUGT to a team number to only see submissions for that team
 */
//#define	DEBUG
//#define	DEBUGT	3
//#define	DEBUG_CMP

/*
 * From the problem statement
 */
#define	MAX_TEAMS	200
#define	MAX_PROBS	20
#define	PENALTY		20

struct STeam {
	int m_nTeam;
	int m_nReject[MAX_PROBS];
	int m_nTime[MAX_PROBS];
	int m_nSubStack[MAX_PROBS];
	int m_nSolve;
	int m_nTotalTime;
} teams[MAX_TEAMS], *pteams[MAX_TEAMS];

/*
 * Compare 2 teams scores.  Args are points to pointers to the team info
 * <0 = p1 is higher ranked (smaller rank #) than p2
 * =0 = p1 same rank as p2
 * >0 = p1 lower ranked (higher rank #) than p1
 */
int cmprank(const void *p1, const void *p2)
{
	STeam *pt1, *pt2;
	int nr, p;

	pt1 = *(STeam **)p1;
	pt2 = *(STeam **)p2;
	/*
	 * Handle # of problems.  If result is <0, it means that pt1 should appear in the
	 * rankings first (IE pt1 has more problems solved)
	 */
	nr = (pt2->m_nSolve - pt1->m_nSolve);
	/*
	 * If zero, then we have to do tie breakers
	 */
	if(nr == 0){
		/*
		 * Compare total time.  Smaller is better.
		 */
		nr = pt1->m_nTotalTime - pt2->m_nTotalTime;
		if(nr == 0){
			/*
		 	 * Starting from last problem solved, work back until we find a difference
			 * Remember, smaller is better here.  Just the opposite of # problems solved.
			 */
			for(p = pt1->m_nSolve-1; p >= 0; p--){
				nr = (pt1->m_nTime[pt1->m_nSubStack[p]] - pt2->m_nTime[pt2->m_nSubStack[p]]);
				if(nr != 0){
#ifdef DEBUG
					::fprintf(stderr, "cmprank: tb%d: nr=%d Team%d(p:%d/t:%d) Team%d(p:%d/t:%d)\n",
						pt1->m_nSolve-p, nr, pt1->m_nTeam, pt1->m_nSubStack[p], pt1->m_nTime[pt1->m_nSubStack[p]],
						pt2->m_nTeam, pt2->m_nSubStack[p], pt2->m_nTime[pt2->m_nSubStack[p]]);
#endif
					break;
				}
			}
			if(nr == 0){
				/* These are truly ranked the same but we order by team number */
				nr = (pt1->m_nTeam - pt2->m_nTeam);
#ifdef DEBUG2
				fprintf(stderr, "cmprank: by team solv=%d Team %d Team %d\n",
					pt2->m_nSolve, pt1->m_nTeam, pt2->m_nTeam);
#endif
				if(nr == 0){
					fprintf(stderr, "Oops!  Can't have cmp result 0 for teams %d and %d!!!\n",
						pt1->m_nTeam, pt2->m_nTeam);
				}
			}
		}
	}
#ifdef DEBUG
	else {
		::fprintf(stderr, "cmprank: #solved nr=%d Team%d(%d) Team%d(%d)\n",
			nr, pt1->m_nTeam, pt1->m_nSolve, pt2->m_nTeam, pt2->m_nSolve);
	}
#endif
	return(nr);
}

/*
 * Compare 2 teams scores.  Args are pointers to teams to compare
 * <0 = p1 is higher ranked (smaller rank #) than p2
 * =0 = p1 same rank as p2
 * >0 = p1 lower ranked (higher rank #) than p1
 */
int cmprank2(STeam *pt1, STeam *pt2)
{
	int nr, p;

	/*
	 * Handle # of problems.  If result is <0, it means that pt1 should appear in the
	 * rankings first (IE pt1 has more problems solved)
	 */
	nr = (pt2->m_nSolve - pt1->m_nSolve);
	/*
	 * If zero, then we have to do tie breakers
	 */
	if(nr == 0){
		/*
		 * Compare total time.  Smaller is better.
		 */
		nr = pt1->m_nTotalTime - pt2->m_nTotalTime;
		if(nr == 0){
			/*
		 	 * Starting from last problem solved, work back until we find a difference
			 * Remember, smaller is better here.  Just the opposite of # problems solved.
			 */
			for(p = pt1->m_nSolve-1; p >= 0; p--){
				nr = (pt1->m_nTime[pt1->m_nSubStack[p]] - pt2->m_nTime[pt2->m_nSubStack[p]]);
				if(nr != 0){
#ifdef DEBUG_CMP
					::fprintf(stderr, "cmprank2: tb%d: nr=%d Team%d(p:%d/t:%d) Team%d(p:%d/t:%d)\n",
						pt1->m_nSolve-p, nr, pt1->m_nTeam, pt1->m_nSubStack[p], pt1->m_nTime[pt1->m_nSubStack[p]],
						pt2->m_nTeam, pt2->m_nSubStack[p], pt2->m_nTime[pt2->m_nSubStack[p]]);
#endif
					break;
				}
			}
			if(nr == 0){
#ifdef DEBUG_CMP
				fprintf(stderr, "cmprank2: same rank teams %d and %d, nsolve=%d time=%d\n",
					pt1->m_nTeam, pt2->m_nTeam,
					pt1->m_nSolve, pt1->m_nTotalTime);
#endif
			}
		}
	}
#ifdef DEBUG_CMP2
	else {
		::fprintf(stderr, "cmprank2: #solved nr=%d Team%d(%d) Team%d(%d)\n",
			nr, pt1->m_nTeam, pt1->m_nSolve, pt2->m_nTeam, pt2->m_nSolve);
	}
#endif
	return(nr);
}

int main(int argc, char **argv)
{
	int nt, np, ns, nr;
	int t, p, ts, d;
	int s, rank, tt, ptt;
	STeam *pt, *ptprev;

	::memset(&(teams[0]), '\0', sizeof(teams));

	if(::scanf("%d %d %d %d", &(nt), &(np), &(ns), &(nr)) != 4){
		::fprintf(stderr, "Bad first line of input\n");
		return(1);
	}
#ifdef DEBUG
	::fprintf(stderr, "NT=%d NP=%d NS=%d NR=%d\n", nt, np, ns, nr);
#endif
	/* Create ptr list of teams - we'll need this for sorting */
	for(t = 0; t < nt; t++){
		teams[t].m_nTeam = t+1;
		pteams[t] = &(teams[t]);
	}
	/* Run the contest */
	for(s = 0; s < ns; s++){
		if(::scanf("%d %d %d %d", &(t), &(p), &(ts), &(d)) != 4){
			::fprintf(stderr, "Read failed on submission %d\n", s);
			return(2);
		}
#if defined(DEBUG) || defined(DEBUGT)
#ifdef DEBUGT
		if(t == DEBUGT)
#endif
		::fprintf(stderr, "Sub %d: Team:%d Prob:%d Time:%d Disp:%d\n",
			s+1, t, p, ts, d);
#endif
		/*
		 * Normalize
		 */
		p--;
		t--;

		pt = pteams[t];
		if(d == 0){
			/* Incorrect submission */
			pt->m_nReject[p]++;
#if defined(DEBUG) || defined(DEBUGT)
#ifdef DEBUGT
			if(pt->m_nTeam == DEBUGT)
#endif
			fprintf(stderr, "Team %d FAILED %d - reject now %d\n", pt->m_nTeam, p+1, pt->m_nReject[p]);
#endif
		} else {
			if(pt->m_nTime[p] == 0){
				pt->m_nTime[p] = pt->m_nReject[p]*PENALTY + ts;
				/* Remember order problems were solved */
				pt->m_nSubStack[pt->m_nSolve] = p;
				pt->m_nTotalTime += pt->m_nTime[p];
#if defined(DEBUG) || defined(DEBUGT)
#ifdef DEBUGT
				if(pt->m_nTeam == DEBUGT)
#endif
				::fprintf(stderr, "Team %d solved %d time=%d  reject=%d\n",
					pt->m_nTeam, p+1, pt->m_nTime[p], pt->m_nReject[p]);
#endif
				pt->m_nSolve++;
			}
		}
	}
	::qsort(&(pteams[0]), nt, sizeof(STeam *), cmprank);

	/*
	 * Print results
	 */
	ptprev = NULL;
	for(t = 0, rank = 1, ptt = -1; t < nt; t++){
		pt = pteams[t];
		if(ptprev != NULL && cmprank2(ptprev, pt) != 0){
			rank = t+1;
			if(rank > nr){
				break;
			}
		}
		tt = pt->m_nTotalTime;
		/*
		 * Set rank if not same as last one
		 */
		ptprev = pt;
		::fprintf(stdout, "%-4d%-4d%3d%5d\n",
			rank, pt->m_nTeam, pt->m_nSolve, tt);
	}
	return(0);
}
