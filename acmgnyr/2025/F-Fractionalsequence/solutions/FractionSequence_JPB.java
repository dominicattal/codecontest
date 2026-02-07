import java.util.Scanner;

public class FractionSequence_JPB {

	public static void main(String[] args) {
		Scanner in = new Scanner(System.in);
		long n = in.nextLong() - 1;
		long m = (long)Math.sqrt(2*n);
		if (m*(m+1)/2 > n)
			m--;
		m++;
		long num = n - (m*(m-1)/2);
		if (num == 0)
			System.out.println(m);
		else {
			long gcd = gcd(m, num);
			System.out.println(m + " " + num/gcd + "/" + m/gcd);
		}
	}

	public static long gcd(long a, long b)
	{
		while (b != 0) {
			long tmp = a;
			a = b;
			b = tmp%b;
		}
		return a;
	}
}
