// Isaac Lee
// 2025 East Division
// Fractional Sequence
// Accepted submission

// The first i blocks consist of the first i(i+1)/2 terms of S, so if we can find the largest integer
// i such that i(i+1)/2 < n, then S(n) = (i+1) (n-i(i+1)/2-1)/(i+1). Using the quadratic formula, it
// turns out that i = ceiling((sqrt(8n+1)-3)/2). The Euclidian algorithm is used to put the answer
// in lowest terms.

import java.io.*;
public class FracSeqIsaac
{
	public static void main(String[] args) throws IOException
	{
		BufferedReader buffy = new BufferedReader(new InputStreamReader(System.in));
		long n = Long.parseLong(buffy.readLine());
		long squarish = (n << 3) + 1L;
		long root = Math.round(Math.sqrt(squarish));
		long square = root * root;
		if(square < squarish)
		{
			root += 2L;
		}
		if(square > squarish)
		{
			root++;
		}
		long i = (root - 3L) >> 1;
		long denominator = i + 1L;
		long numerator = n - ((i * denominator) >> 1) - 1L;
		if(numerator == 0)
		{
			System.out.println(denominator);
		}
		else
		{
			long greatest = gcd(numerator,denominator);
			System.out.println(denominator + " " + (numerator / greatest) + "/" + (denominator / greatest));
		}
	}
	public static long gcd(long a,long b)
	{
		while(a > 0)
		{
			long temp = b;
			b = a;
			a = temp % a;
		}
		return b;
	}
}
