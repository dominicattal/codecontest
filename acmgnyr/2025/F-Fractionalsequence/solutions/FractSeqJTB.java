// East Division Regional 2025
// Fractional Sequence Solution
// John Buck

import java.util.Scanner;

class FractSeqJTB {
	/*
	 * Set to 'true' to reduce fractions
	 */
	private static final boolean REDUCE_FRACTION = true;

	private static long[] reduceFraction(long num, long den) {
		long div = gcd(num, den);
		long rNum = (num / div);
		long rDen = (den / div);
		return new long[] {rNum, rDen};
	}
	private static long gcd(long a, long b) {
		while(b != 0){
			long t = b;
			b = a % b;
			a = t;
		}
		return(a);
	}

	public static void main(String [] args) {
		Scanner stdin = new Scanner(System.in);
		double dblidx;
		long N, idx;

		N = Long.parseLong(stdin.next());

		/*
		 * Normalize to 0 based index
		 */
		N--;

		/*
		 * We'll use the Quadratic formula: (-b + sqrt(b^2 - 4*a*c))/2*a
		 * to solve for the value at the start of the block for index "N"
		 * "N" group starts at 1 + (idx-1)*idx/2 = N
		 * 1 + (idx^2 - idx)/2 = N
		 * 2 + (idx^2 - idx) = 2 * N
		 * 2 + (idx^2 - idx) - 2 * N = 0
		 * (-1*(-1) + Math.sqrt(1 - 4*1*(-2*N))) / (2*1)
		 */
		dblidx = (1.0 + Math.sqrt(1.0 - 4.0*1.0*(-2.0*N))) / (2.0*1.0);
		/*
		 * integer version, which is index of first thing in N group
		 */
		idx = (long)dblidx;

		/*
		 * Subtract off the value at the start of the group.  This will
		 * give us the sub-index into the group
		 */
		N -= ((idx-1) * idx)/2;

		/*
		 * If remainder, it's the sub-index into the block, eg, fractional numerator
		 */
		if(N != 0){
			if(!REDUCE_FRACTION){
				System.out.println("" + idx + " " + N + "/" + idx);
			} else {
				long [] newFract = reduceFraction(N, idx);
				System.out.println("" + idx + " " + newFract[0] + "/" + newFract[1]);
			}
		} else {
			/*
			 * No fractional part
			 */
			System.out.println("" + idx);
		}
	}
}
