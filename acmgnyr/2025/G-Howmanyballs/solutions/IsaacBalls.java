// Isaac Lee
// 2025 East Division
// How Many Balls?
// Accepted submission

// This solution tries every value of r from 1 to 10^6, for each one finding the g such that
// Prob2Diff(r,g) = p/q and checking whether it's an integer >= r. Either the first r and g that work
// are output, or it is determined that no r within bounds works.

import java.io.*;
public class IsaacBalls
{
	public static void main(String[] args) throws IOException
	{
		BufferedReader buffy = new BufferedReader(new InputStreamReader(System.in));
		String[] pq = buffy.readLine().split(" ");
		long p = Long.parseLong(pq[0]);
		long q = Long.parseLong(pq[1]);
		boolean surchin = true;
		for(long r = 1;r < 1000001 && surchin;r++)
		{
			long pplustwoqr = p + ((q * r) << 1);
			long square = pplustwoqr * pplustwoqr - ((p * q * r * r) << 3);
			long root = Math.round(Math.sqrt(square));
			if(root * root == square)
			{
				long left = pplustwoqr - ((p * r) << 1);
				long denoom = p << 1;
				long noom = left - root;
				if(noom > 0 && noom % denoom == 0)
				{
					long g = noom / denoom;
					System.out.println(r + " " + g);
					surchin = false;
					break;
				}
				noom = left + root;
				if(noom % denoom == 0)
				{
					long g = noom / denoom;
					System.out.println(r + " " + g);
					surchin = false;
				}
			}
		}
		if(surchin)
		{
			System.out.println("impossible");
		}
	}
}
