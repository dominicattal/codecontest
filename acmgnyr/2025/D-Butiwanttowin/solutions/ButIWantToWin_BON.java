import java.util.Scanner;

public class ButIWantToWin_BON {

	public static final int MAXC = 20;
	
	public static long[] votes;
	public static void main(String [] args)
	{
		Scanner in = new Scanner(System.in);
		int c = in.nextInt();
		votes = new long[c];
		votes[0] = in.nextLong();
		long total = votes[0];
		for(int i=1; i<c; i++) {
			long val = in.nextLong();
			int j=i;
			while (j>0 && votes[j-1] > val) {
				votes[j] = votes[j-1];
				j--;
			}
			votes[j] = val;
			total += val;
		}
		if (votes[c-1]*2 >= total) {
			System.out.println("IMPOSSIBLE TO WIN");
			System.exit(0);
		}
		
		int j=1;					// index of first candidate without min # of first-place votes
		int round = 0;
		long vtd = votes[0];		// vtd = votes to distribute
		while (true) {
			round++;
											// check if we have enough votes for 2nd place to win
			if (vtd + votes[c-2] > total/2) {
				System.out.println(round);
				break;
			}
											// determine how many ties we can make
			long r = votes[j];
			int numtied = 1;
			while (j<c-1 && r + vtd >= numtied*votes[j+1]) {
				j++;
				numtied++;
				r += votes[j];
			}
			vtd += r;
			j++;
		}
	}
}
