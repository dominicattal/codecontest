// Isaac Lee
// 2025 East Division
// Number Pyramid
// Accepted submission

// This solution simulates the procedure described in the problem, repeatedly scanning the rows top
// to bottom, from left to right within each row, looking at the up to 3 triangles each position is
// part of and filling them in if possible. The process finishes when we do a full scan without making
// changes, at which point we go through the board looking for contradictions and empty spaces to
// determine the answer. The space complexity is O(n^2) and since in the worst case we could only
// fill one position per full scan, the time complexity is O(n^4).

import java.io.*;
public class NumberIsaac
{
	public static void main(String[] args) throws IOException
	{
		BufferedReader buffy = new BufferedReader(new InputStreamReader(System.in));
		int n = Integer.parseInt(buffy.readLine());
		int[][] pyramid = new int[n][];
		boolean contradiction = false;
		for(int index = 0;index < n;index++)
		{
			pyramid[index] = new int[index + 1];
			String[] row = buffy.readLine().split(" ");
			for(int jindex = 0;jindex <= index;jindex++)
			{
				if((pyramid[index][jindex] = Integer.parseInt(row[jindex])) == -100)
				{
					contradiction = true;
				}
			}
		}
		boolean changed = true;
		while(changed && !contradiction)
		{
			changed = false;
			for(int index = 0;index < n && !contradiction;index++)
			{
				for(int jindex = 0;jindex <= index && !contradiction;jindex++)
				{
					if(pyramid[index][jindex] == 100)
					{
						boolean justchanged = false;
						if(index < n - 1 && pyramid[index + 1][jindex] < 100 && pyramid[index + 1][jindex + 1] < 100)
						{
							pyramid[index][jindex] = pyramid[index + 1][jindex] + pyramid[index + 1][jindex + 1];
							justchanged = (changed = true);
						}
						else
						{
							if(jindex < index && pyramid[index - 1][jindex] < 100 && pyramid[index][jindex + 1] < 100)
							{
								pyramid[index][jindex] = pyramid[index - 1][jindex] - pyramid[index][jindex + 1];
								justchanged = (changed = true);
							}
							else if(jindex > 0 && pyramid[index - 1][jindex - 1] < 100 && pyramid[index][jindex - 1] < 100)
							{
								pyramid[index][jindex] = pyramid[index - 1][jindex - 1] - pyramid[index][jindex - 1];
								justchanged = (changed = true);
							}
						}
						if(justchanged && (pyramid[index][jindex] < -99 || pyramid[index][jindex] > 99))
						{
							contradiction = true;
						}
					}
				}
			}
		}
		boolean unsolvable = false;
		for(int index = 0;index < n;index++)
		{
			for(int jindex = 0;jindex <= index;jindex++)
			{
				if(index < n - 1 && pyramid[index][jindex] < 100 && pyramid[index + 1][jindex] < 100 && pyramid[index + 1][jindex + 1] < 100 && pyramid[index][jindex] != pyramid[index + 1][jindex] + pyramid[index + 1][jindex + 1])
				{
					contradiction = true;
				}
				if(pyramid[index][jindex] == 100)
				{
					unsolvable = true;
				}
			}
		}
		if(contradiction)
		{
			System.out.println("no solution");
		}
		else
		{
			if(unsolvable)
			{
				System.out.println("ambiguous");
			}
			else
			{
				StringBuilder result = new StringBuilder();
				result.append("solvable\n");
				for(int index = 0;index < n;index++)
				{
					for(int jindex = 0;jindex <= index;jindex++)
					{
						result.append(pyramid[index][jindex]);
						if(jindex == index)
						{
							result.append("\n");
						}
						else
						{
							result.append(" ");
						}
					}
				}
				System.out.print(result);
			}
		}
	}
}
