import java.util.Scanner;

public class NumberPyramid {

	public static final int EMPTY = 100000;

	public static int[][] card;

	public static void main(String[] args) {

		Scanner in = new Scanner(System.in);
		int n = in.nextInt();
		card = new int[n][n];
		for(int i=0; i<n; i++) {
			for(int j=0; j<=i; j++) {
				card[i][j] = in.nextInt();
				if (card[i][j] == 100)
					card[i][j] = EMPTY;
			}
		}
		switch (process(n)) {
		case 0:
			System.out.println("solvable");
			for(int i=0; i<n; i++) {
				System.out.print(card[i][0]);
				for(int j=1; j<=i; j++)
					System.out.printf(" " + card[i][j]);
				System.out.println();
			}
			break;
		case 1:
			System.out.println("ambiguous");
			break;
		case 2:
			System.out.println("no solution");
		}
	}

	public static int process(int n)
	{
		boolean changesMade = true;
		while (changesMade) {
			changesMade = false;
			for(int i=0; i<n; i++) {
				for(int j=0; j<=i; j++) {
					if (card[i][j] == EMPTY){
						if (j > 0 && card[i-1][j-1] != EMPTY && card[i][j-1] != EMPTY) {
							card[i][j] = card[i-1][j-1] - card[i][j-1];
							changesMade = true;
						}
						else if (j < i && card[i-1][j] != EMPTY && card[i][j+1] != EMPTY) {
							card[i][j] = card[i-1][j] - card[i][j+1];
							changesMade = true;
						}
						else if (i < n-1 && card[i+1][j] != EMPTY && card[i+1][j+1] != EMPTY) {
							card[i][j] = card[i+1][j] + card[i+1][j+1];
							changesMade = true;
						}
					}
				}
			}
		}

		boolean foundEmpty = false;
		for(int i=0; i<n; i++) {
			for(int j=0; j<=i; j++) {
				if (card[i][j] != EMPTY && (card[i][j] >= 100 || card[i][j] <= -100))
					return 2;
				else if (card[i][j] == EMPTY)
					foundEmpty = true;
				else if (i < n-1 && card[i+1][j] != EMPTY && card[i+1][j+1] != EMPTY && card[i][j] != card[i+1][j] + card[i+1][j+1])
					return 2;
			}
		}
		return (foundEmpty ? 1 : 0);
	}
}
