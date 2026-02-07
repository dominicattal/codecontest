/*
 * Left over pizza.
 * John Buck
 * East Division Regional 2025
 */
#include <stdio.h>
#include <memory.h>
int main()
{
	int n, slices;
	char szType[32];
	int tot[4];
	memset(&(tot[0]), '\0', sizeof(tot));
	fscanf(stdin, "%d", &(n));
	while(n--) {
		fscanf(stdin, "%s %d", &(szType[0]), &(slices));
		tot[szType[0] & 3] += slices;
	}
	printf("%d\n", (tot[3] + 5) / 6 + (tot[1] + 7) / 8 + (tot[0] + 11) / 12);
	return(0);
}
