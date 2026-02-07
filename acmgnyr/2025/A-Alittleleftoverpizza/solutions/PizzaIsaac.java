// Isaac Lee
// 2025 East Division
// Leftover Pizza
// Accepted submission

// A straightforward solution which counts the number of slices for each size of pizza, then ceiling
// divides each count by the respective number of slices per box and adds these ratios up.

import java.io.*;
public class PizzaIsaac
{
	public static void main(String[] args) throws IOException
	{
		BufferedReader buffy = new BufferedReader(new InputStreamReader(System.in));
		short n = Short.parseShort(buffy.readLine());
		short smalls = 0;
		short mediums = 0;
		short larges = 0;
		for(short i = 0;i < n;i++)
		{
			String[] sili = buffy.readLine().split(" ");
			char si = sili[0].charAt(0);
			short li = Short.parseShort(sili[1]);
			if(si == 'S')
			{
				smalls += li;
			}
			if(si == 'M')
			{
				mediums += li;
			}
			if(si == 'L')
			{
				larges += li;
			}
		}
		System.out.println((smalls + 5) / 6 + (mediums + 7) / 8 + (larges + 11) / 12);
	}
}

