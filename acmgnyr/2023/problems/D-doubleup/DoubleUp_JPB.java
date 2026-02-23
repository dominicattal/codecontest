import java.math.BigInteger;
import java.util.Scanner;

public class DoubleUp_JPB {

	public static final int SIZE = 1000;
	
	public static int[][] table = new int[SIZE][SIZE+1];
	
	public static void main(String[] args) {
		Scanner in = new Scanner(System.in);
		int n = in.nextInt();
		double ln2 = Math.log(2.0);
		for(int i=0; i<n; i++) {
			double val = in.nextDouble();
			table[i][1] = (int)(Math.log(val)/ln2 + 0.1);
//			System.out.println(table[i][1]);
		}
		for(int len=2; len<=n; len++) {
			for(int i=0; i<=n-len; i++) {
				int best = Math.max(table[i][len-1], table[i+1][len-1]);
				for(int j=1; j<=len-1; j++) {
					if (table[i][j] == table[i+j][len-j]) {
						if (table[i][j]+1 > best)
							best = table[i][j]+1;
					}
				}
				table[i][len] = best;
			}
		}
//		System.out.println(table[0][n]);
		BigInteger b = new BigInteger("2");
		b = b.pow(table[0][n]);
		System.out.println(b);
	}

}
