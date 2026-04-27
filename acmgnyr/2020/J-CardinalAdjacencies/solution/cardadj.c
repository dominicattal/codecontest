/*
 * Cardinal Adjacencies
 * Author: John Buck
 * ICPC GNYR 2020 Regional Contest
 */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#define	MAXRC	10000
#define	LAND	1

/*
 * Only need to remember current row and previous
 * We will keep 2 pointers around.  One for the current row and one for the previous row.
 * This will allow on-the-fly checks for adjacency.  After each loop, we'll swap the pointers
 * so the current becomes previous, and the previous one will get overwritten with the next
 * row of data.
 */
unsigned char rows[2][MAXRC];

int main(int argc, char **argv)
{
	unsigned char *pPrev = &(rows[0][0]);
	unsigned char *pCur = &(rows[1][0]);
	unsigned char *pTmp = pCur;
	int nr, nc;
	int r, c;
	int nca, nica;
	int val;

	/*
	 * Get rows/cols
	 */
	if(scanf("%d %d", &(nr), &(nc)) != 2){
		fprintf(stderr, "Can not get rows and columns\n");
		return(1);
	}
	nca = nica = 0;
	/*
	 * Read in the data and process as we read
	 */
	for(r = 0; r < nr; r++){
		for(c = 0; c < nc; c++, pTmp++){
			if(scanf("%d", &(val)) != 1){
				fprintf(stderr, "Out of data on row %d col %d\n", r, c);
				return(2);
			}
			/*
			 * Sanity check on input data
			 */
			if(val < 0 || val > 1){
				fprintf(stderr, "Bad value %d on row %d col %d\n", val, r, c);
				return(3);
			}
			/*
			 * pTmp moves down each element in the current row
			 */
			*pTmp = (unsigned char)val;
			/*
			 * Only care about land
			 */
			if(val == LAND){
				/*
				 * If not first column, we can check left neighbor and up left neighbor
				 * Note the first time through, pPrev points to all 0's
				 */
				if(c > 0){
					if(pTmp[-1] == LAND){
						nca++;
					}
					if(pPrev[c-1] == LAND){
						nica++;
					}
				}
				/*
				 * If neighbor above us is land
				 */
				if(pPrev[c] == LAND){
					nca++;
				}
				/*
				 * If we're not on the last element in the row, check our up right neighbor
				 */
 				if(c < nc-1 && pPrev[c+1] == LAND){
					nica++;
				}
			}
		}
		/*
		 * Swap prev and current rows, using pTmp as a swap temporary
		 */
		pTmp = pPrev;
		pPrev = pCur;
		pCur = pTmp;
	}
	/*
	 * Print answer.  Note that nica has nca added to it here
	 */
	printf("%d %d\n", nca, nica+nca);
	return(0);
}
