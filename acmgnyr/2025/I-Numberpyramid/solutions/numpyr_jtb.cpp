/*
 * Number Pyramid - East Division Regional 2025
 * John Buck, GNY Region
 * 
 */
#include <stdio.h>
int rows[101][101];

int main()
{
    int i, j, n, emptyCnt;
	bool changed = true;
	bool testContradiction;

	emptyCnt = ::fscanf(stdin, "%d", &(n));
	for(i = 1; i <= n; i++){
		for(j = 1; j <= i; j++){
			emptyCnt = ::fscanf(stdin, "%d", &(rows[i][j]));
		}
	}
	while(changed){
		changed = false;
		emptyCnt = 0;
		for(i = 1; i < n; i++){
			for(j = 1; j <= i; j++){
				if(rows[i][j] == 100){
					if(rows[i+1][j] < 100 && rows[i+1][j+1] < 100){
						rows[i][j] = rows[i+1][j] + rows[i+1][j+1];
						if(rows[i][j] >= 100){
							fprintf(stdout, "no solution\n");
							return(0);
						}
						changed = true;
					} else {
						emptyCnt++;
					}
				}
				if(rows[i][j] != 100){
					testContradiction = false;
					if(rows[i+1][j] == 100){
						if(rows[i+1][j+1] != 100){
							rows[i+1][j] = rows[i][j] - rows[i+1][j+1];
							changed = true;
							testContradiction = true;
						}
					} else if(rows[i+1][j+1] == 100){
						if(rows[i+1][j] != 100){
							rows[i+1][j+1] = rows[i][j] - rows[i+1][j];
							changed = true;
							testContradiction = true;
						}
					} else {
						testContradiction = true;
					}
					if(changed){
						if(rows[i][j] >= 100){
							fprintf(stdout, "no solution\n");
							return(0);
						}
					}
					if(testContradiction && rows[i+1][j] + rows[i+1][j+1] != rows[i][j]){
						fprintf(stdout, "no solution\n");
						return(0);
					}
				}
			}
		}
	}
	/*
	 * Check last row for empties
	 */
	for(j = 1; j <= n; j++){
		if(rows[n][j] == 100){
			emptyCnt++;
			break;
		}
	}
	if(emptyCnt == 0){
		fprintf(stdout, "solvable\n");
		for(i = 1; i <= n; i++){
			for(j = 1; j <= i; j++){
				fprintf(stdout, "%d ", rows[i][j]);
			}
			fputc('\n', stdout);
		}
	} else {
		fprintf(stdout, "ambiguous\n");
	}
	return(0);
}
