// Isaac Lee
// 2025 East Division
// Chess Solitaire
// Accepted submission

// This solution uses a recursive backtracking approach, at each stage making the lexicographically
// least move and stopping at positions which don't have moves.

import java.io.*;
import java.util.*;
public class IsaacSolitaire
{
	public static void main(String[] args) throws IOException
	{
		BufferedReader buffy = new BufferedReader(new InputStreamReader(System.in));
		String[] nandm = buffy.readLine().split(" ");
		int n = Integer.parseInt(nandm[0]);
		int m = Integer.parseInt(nandm[1]);
		char[][] board = new char[n][n];
		for(int whatever = 0;whatever < m;whatever++)
		{
			String[] pandloc = buffy.readLine().split(" ");
			String loc = pandloc[1];
			board[loc.charAt(0) - 65][loc.charAt(1) - 49] = pandloc[0].charAt(0);
		}
		int[] posmoves = new int[m + 1];
		char[] piecemoves = new char[m + 1];
		if(recurse(board,m,posmoves,piecemoves))
		{
			StringBuilder result = new StringBuilder();
			for(int index = m;index > 1;index--)
			{
				result.append(piecemoves[index]);
				result.append(": ");
				int move = posmoves[index];
				result.append((char)((move >> 9) + 65));
				result.append(((move >> 6) & 7) + 1);
				result.append(" -> ");
				result.append((char)(((move >> 3) & 7) + 65));
				result.append((move & 7) + 1);
				result.append("\n");
			}
			System.out.print(result);
		}
		else
		{
			System.out.println("No solution");
		}
	}
	public static boolean recurse(char[][] board,int numpieces,int[] posmoves,char[] piecemoves)
	{
		if(numpieces == 1)
		{
			return true;
		}
		for(int fromrow = 0;fromrow < board.length;fromrow++)
		{
			for(int fromcol = 0;fromcol < board.length;fromcol++)
			{
				char movedpiece = board[fromrow][fromcol];
				if(movedpiece > 0)
				{
					int[] moves = new int[8];
					int lenmoves = 0;
					int[][] diffs = new int[][]{{-1,0},{0,1},{1,0},{0,-1}};
					if(movedpiece == 'Q' || movedpiece == 'K')
					{
						diffs = new int[][]{{-1,0},{-1,1},{0,1},{1,1},{1,0},{1,-1},{0,-1},{-1,-1}};
					}
					if(movedpiece == 'N')
					{
						diffs = new int[][]{{-2,1},{-1,2},{1,2},{2,1},{2,-1},{1,-2},{-1,-2},{-2,-1}};
					}
					if(movedpiece == 'B')
					{
						diffs = new int[][]{{-1,1},{1,1},{1,-1},{-1,-1}};
					}
					for(int[] diff:diffs)
					{
						int rowdiff = diff[0];
						int coldiff = diff[1];
						int torow = fromrow + rowdiff;
						int tocol = fromcol + coldiff;
						if(movedpiece != 'N' && movedpiece != 'K')
						{
							while(torow > -1 && torow < board.length && tocol > -1 && tocol < board.length && board[torow][tocol] == 0)
							{
								torow += rowdiff;
								tocol += coldiff;
							}
						}
						if(torow > -1 && torow < board.length && tocol > -1 && tocol < board.length && board[torow][tocol] > 0)
						{
							moves[lenmoves++] = (torow << 3) | tocol;
						}
					}
					Arrays.sort(moves);
					for(int index = 8 - lenmoves;index < 8;index++)
					{
						int move = moves[index];
						int torow = move >> 3;
						int tocol = move & 7;
						board[fromrow][fromcol] = 0;
						char capiece = board[torow][tocol];
						board[torow][tocol] = movedpiece;
						posmoves[numpieces] = (fromrow << 9) | (fromcol << 6) | (torow << 3) | tocol;
						piecemoves[numpieces] = movedpiece;
						if(recurse(board,numpieces - 1,posmoves,piecemoves))
						{
							return true;
						}
						board[torow][tocol] = capiece;
						board[fromrow][fromcol] = movedpiece;
					}
				}
			}
		}
		return false;
	}
}

