/*
 * East Division Regional 2025
 * John Buck, Greater NY Region
 * 
 * Ranked Choice Voting
 *
 * Lots of debugging code could be removed, but may be helpful to those reading this code
 */
//#define _DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_CAND    20

struct SCandidate {
    int m_nCand;
    int m_nVotes;
};
int nVoters;
int nCandidates;
int nMajority;

SCandidate cands[MAX_CAND];

int cmpvotes(const void *p1, const void *p2)
{
    SCandidate *pc1 = (SCandidate *)p1;
    SCandidate *pc2 = (SCandidate *)p2;
    return(pc2->m_nVotes - pc1->m_nVotes);
}

#ifdef _DEBUG
int nLeader;
int nRunnerUp;

void ShowCandidates()
{
    int i;
    for (i = 0; i < nCandidates; i++) {
        ::fprintf(stderr, "%4d: %4d votes\n", cands[i].m_nCand, cands[i].m_nVotes);
    }
}
#endif

/* Count votes left besides first 1 candidates */
int CountVotesLeft()
{
    int i, nCount = 0;
    for (i = 2; i < nCandidates; i++) {
        nCount += cands[i].m_nVotes;
    }
#ifdef _DEBUG
    ::fprintf(stderr, "%s: There are a total %d votes left to distribute with %d candidates left\n",
        __FUNCTION__, nCount, nCandidates);
#endif
    return(nCount);
}

int main()
{
    int i;
    int nExtra;
    int nNeeded;
    int nVotesLeft;
    int nMinVotes;
	int nGroup;
	int nGroupVotes;

    if (::scanf("%d", &(nCandidates)) != 1) {
        ::fprintf(stderr, "can't get voters and candidates\n");
        return(0);
    }
    if (nCandidates > MAX_CAND) {
        ::fprintf(stderr, "Too many candidates (%d) max is %d\n", nCandidates, MAX_CAND);
        return(0);
    }
    nVoters = 0;
    for (i = 0; i < nCandidates; i++) {
        if (::scanf("%d", &(cands[i].m_nVotes)) != 1) {
            ::fprintf(stderr, "error reading votes for candidate %d\n", i);
            return(0);
        }
        cands[i].m_nCand = i+1;
        nVoters += cands[i].m_nVotes;
    }
#ifdef _DEBUG
    ShowCandidates();
#endif
    /* For an odd number, the majority is one more */
    nMajority = nVoters / 2 + 1;
    ::qsort(&(cands[0]), nCandidates, sizeof(SCandidate), cmpvotes);

#ifdef _DEBUG
    nLeader = cands[0].m_nCand;
    nRunnerUp = cands[1].m_nCand;
    ::fprintf(stderr, "There are %d voters, the majority is %d votes.\n", nVoters, nMajority);
    ::fprintf(stderr, "The current leader is candidate %d with %d votes, runner up is candidate %d with %d votes\n",
        nLeader, cands[0].m_nVotes, nRunnerUp, cands[1].m_nVotes);
    ShowCandidates();
#endif
    /* How many votes needed for candidate 2 to win */
    nNeeded = nMajority - cands[1].m_nVotes;
    nVotesLeft = CountVotesLeft();
#ifdef _DEBUG
    ::fprintf(stderr, "Runner-up (%d) needs %d votes for majority - there are a total of %d votes available\n",
        nRunnerUp, nNeeded, nVotesLeft);
#endif
	/*
	 * Just give up now if not enough votes to make a difference
	 */
	if(nVotesLeft < nNeeded){
		::fprintf(stdout, "IMPOSSIBLE TO WIN\n");
		return(0);
	}
    /*
     * We assume that every voter who didn't vote for the leader or runner-up as their first choice, ranked their votes in reverse order and
     * left the leader and runner-up as their last choices.  This would give the minimum number of votes required.  Basically, it comes down
     * to 3 candidates.
     */
    nMinVotes = cands[--nCandidates].m_nVotes;;
#ifdef _DEBUG
	::fprintf(stderr, "Start rounds: nMinVotes=%d nCandidates=%d\n", nMinVotes, nCandidates);
#endif
    for (i = 1; nCandidates > 2; i++) {
        /* Get last place candidate's votes */
		nExtra = cands[--nCandidates].m_nVotes;
		nGroup = 1;
		nGroupVotes = cands[nCandidates-1].m_nVotes;
#ifdef _DEBUG
		::fprintf(stderr, "   i=%d nExtra=%d nGroupVotes=%d\n", i, nExtra, nGroupVotes);
#endif
		while(nCandidates > 2 && nExtra + nMinVotes >= nGroupVotes){
#ifdef _DEBUG
			::fprintf(stderr, "        Group ties: nCandidates=%d nExtra=%d nMinVotes=%d nGroupVotes=%d\n",
				nCandidates, nExtra, nMinVotes, nGroupVotes);
#endif
			nCandidates--;
			nGroup++;
			nExtra += cands[nCandidates].m_nVotes;
			nGroupVotes = nGroup * cands[nCandidates-1].m_nVotes;
		}
        nMinVotes += nExtra;
#ifdef _DEBUG
		::fprintf(stderr, "   After group ties: nCandidates=%d nMinVotes=%d nExtra=%d nGroup=%d nGroupVotes=%d\n",
			nCandidates, nMinVotes, nExtra, nGroup, nGroupVotes);
#endif
	}
    ::fprintf(stdout, "%d\n", i);

    return(0);
}

