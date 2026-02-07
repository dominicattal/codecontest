// Isaac Lee
// 2025 East Division
// Andor Strikes Again
// Accepted submission

// For a leaf node, the minimum number of changes to change the subtree rooted there is clearly 1.
// For internal nodes, it depends whether they're or/and and initially true/false. For a true or node,
// at least one of its children must be true, so all of these must be changed to false in order to
// make it false, so the minimum number of changes to change it is the sum of the minimum changes of
// its children which are initially true. For a false or node, all of its children must currently be
// false, so its minimum number of changes is the minimum number of changes over all its children.
// Similarly, the minimum changes for true and nodes are the minimum over their children and for false
// and nodes, it's the sum over nodes which are currently false. This solution uses a bottom-up
// approach, finding the minimum number of changes to flip each subtree, from the leaves up to the
// root. The time and space complexities are proportional to the number of nodes in the tree.

import java.io.*;
public class AndorIsaac
{
	public static void main(String[] args) throws IOException
	{
		BufferedReader buffy = new BufferedReader(new InputStreamReader(System.in));
		String[] nandt = buffy.readLine().split(" ");
		int n = Integer.parseInt(nandt[0]);
		char t = nandt[1].charAt(0);
		boolean or = (t == 'O' && ((n & 1) == 1)) || (t == 'A' && ((n & 1) == 0));
		String[][] rows = new String[n][];
		boolean[][] values = new boolean[n][];
		int[][] minchanges = new int[n][];
		for(int index = 0;index < n;index++)
		{
			rows[index] = buffy.readLine().split(" ");
			values[index] = new boolean[rows[index].length];
			minchanges[index] = new int[rows[index].length];
		}
		for(int index = n - 1;index > -1;index--)
		{
			int kindex = 0;
			for(int jindex = 0;jindex < rows[index].length;jindex++)
			{
				char furst = rows[index][jindex].charAt(0);
				if(furst == 'T' || furst == 'F')
				{
					values[index][jindex] = furst == 'T';
					minchanges[index][jindex] = 1;
				}
				else
				{
					int lindex = kindex;
					int numchildren = Integer.parseInt(rows[index][jindex]);
					boolean value = false;
					while(kindex < lindex + numchildren)
					{
						value = value || values[index + 1][kindex] == or;
						kindex++;
					}
					if(!or)
					{
						value = !value;
					}
					values[index][jindex] = value;
					int minchange = 1000000;
					if(or == value)
					{
						minchange = 0;
					}
					while(lindex < kindex)
					{
						int change = minchanges[index + 1][lindex];
						if(or == value)
						{
							if(value == values[index + 1][lindex])
							{
								minchange += change;
							}
						}
						else
						{
							minchange = Math.min(minchange,change);
						}
						lindex++;
					}
					minchanges[index][jindex] = minchange;
				}
			}
			or = !or;
		}
		System.out.println(minchanges[0][0]);
	}
}

