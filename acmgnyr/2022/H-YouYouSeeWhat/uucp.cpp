/*
 * D - You You See what?
 * Greater NY Regional
 * Author: John Buck
 * Longer than it has to be for clarity.
 */
//#define	DEBUG
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#define	MAXCOMP	256

char *comps[MAXCOMP];
int ncomp = 0;

int main(int argc, char **argv)
{
	char *s, *user;
	int i, j, nsame;
	char szBuf[512];

	if(::fgets(&(szBuf[0]), sizeof(szBuf), stdin) != NULL){
		// break into components
		comps[ncomp++] = &(szBuf[0]);
		for(s = &(szBuf[0]); isalnum(*s) || *s == '!'; s++){
			if(*s == '!'){
				*s++ = '\0';
				comps[ncomp++] = s;
			}
		}
		*s = '\0';
#ifdef DEBUG
		for(i = 0; i < ncomp; i++){
			printf("%d: %s\n", i, comps[i]);
		}
#endif

		// strip off user since it's not a hop
		user = comps[ncomp-1];
		ncomp--;
		// eliminate hops to self
		for(i = 0; i < ncomp; i++){
			// count # of same
			for(j = i+1; j < ncomp; j++){
				if(::strcasecmp(comps[i], comps[j]) != 0){
					break;
				}
			}
			// how many are the same?
			nsame = j - i - 1;
			if(nsame > 0){
				while(j < ncomp){
					comps[j-nsame] = comps[j];
					j++;
				}
				ncomp -= nsame;
			}
		}
#ifdef DEBUG
		printf("after removing hops to self, ncomp=%d:\n", ncomp);
		for(i = 0; i < ncomp; i++){
			printf("%d: %s\n", i, comps[i]);
		}
#endif
		// if there are more than 2 hops then we may be able to trim
		if(ncomp > 2){
#ifdef DEBUG
			printf("ncomp = %d\n", ncomp);
#endif
			i = 1;
			while(i < ncomp-1){
#ifdef DEBUG
				printf(" loop i=%d ncomp=%d\n", i, ncomp);
#endif
				if(::strcasecmp(comps[i-1], comps[i+1]) == 0){
#ifdef DEBUG
					printf("   deleting %s and %s at index %d\n", comps[i], comps[i+1], i);
#endif
					// going to the same place we came from, so delete this and next
					// by copying remaining elements
					ncomp -= 2;
#ifdef DEBUG
					printf("   ncomp now %d\n", ncomp);
#endif
					for(j = i; j < ncomp; j++){
#ifdef DEBUG
						printf("     Copy %s at index %d to index %d\n", comps[j+2], j+2, j);
#endif
						comps[j] = comps[j+2];
					}
					if(i > 1){
						i--;
#ifdef DEBUG
						printf("   decrement i to %d\n", i);
#endif
					}
				} else {
					i++;
#ifdef DEBUG
					printf("   increment i to %d\n", i);
#endif
				}
			}
		}
		for(i = 0; i < ncomp; i++){
			printf("%s!", comps[i]);
		}
		// always print user
		printf("%s\n", user);
	}
	return(0);
}
	
