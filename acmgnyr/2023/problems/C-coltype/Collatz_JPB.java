import java.util.Scanner;

public class Collatz_JPB {

	
	public static void main(String [] args)
	{
		Scanner in = new Scanner(System.in);
		String s = in.next();
		int n = s.length();
		long b = 1, c = 3;
		boolean valid = true;
		if (s.charAt(n-1) != 'O')
			valid = false;
		else {
/*												first odd has to have form (4^n-1)/3
 * 												have every term in series have form (a4^n-b)/c where initially
 * 													a=1, b=1, c=3
 * 												working backward, each E makes b = 2*b
 * 																  each O makes b = b+c, c = 3*c
 * 													(don't have to worry about the a's)
 */
			boolean prevO = true;
			for(int i=n-2; i>=0; i--) {
				char ch = s.charAt(i);
				if (ch == 'E') {
					prevO = false;
					b *= 2;
				}
				else if (ch == 'O') {
					if (prevO) {
						valid = false;
						break;
					}
					prevO = true;
					b += c;
					c *= 3;
				}
				else {						// shouldn't happen
					valid = false;
					break;
				}
			}
		}
		if (!valid)
			System.out.println("INVALID");
		else {
			long a = 2;
			while (a-b <c)
				a *= 2;
			long rem = b % c;
			int count = 1;
			while ((a%c != rem || power2((a-b)/c)) && count < 50) {
				a *= 2;
				count++;
			}
			if (a%c == rem)
				System.out.println((a-b)/c);
			else
				System.out.println("WHO KNOWS");
			
		}
	}
	
	public static boolean power2(long a)
	{
		long exp = (long)(Math.log(a)/Math.log(2) + 0.000001);
		return a == Math.pow(2, exp);
	}
}
