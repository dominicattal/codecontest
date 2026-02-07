import java.util.ArrayList;
import java.util.Scanner;

public class ChessSolitaire_Bon {

	public static class MoveTo
	{
		public int row, col;

		public MoveTo(int row, int col)
		{
			this.row = row;
			this.col = col;
		}
	}

	public static class Move
	{
		public char piece;
		public int row, col, fromRow, fromCol;

		public Move(char piece, int row, int col, int fromRow, int fromCol)
		{
			this.piece = piece;
			this.row = row;
			this.col = col;
			this.fromRow = fromRow;
			this.fromCol = fromCol;

		}
	}
	public static char[][] board;

	public static void main(String[] args) {

		Scanner in = new Scanner(System.in);
		int size = in.nextInt();
		board = new char[size+2][size+2];
		for(int r=0; r<board.length; r++)
			for(int c=0; c<board.length; c++)
				board[r][c] = ' ';
		int npieces = in.nextInt();
		for(int i=0; i<npieces; i++) {
			char ch = in.next().charAt(0);
			String pos = in.next();
			int row = pos.charAt(0) - 'A' + 1;
			int col = pos.charAt(1) - '0';
			board[row][col] = ch;
		}
		ArrayList<Move> solution = new ArrayList<>();
		if (solve(npieces, solution)) {
			for(Move m: solution) {
				System.out.println(m.piece + ": " + convert(m.fromRow, m.fromCol) + " -> " + convert(m.row,+ m.col));			
			}
		}
		else
			System.out.println("No solution");
	}

	public static boolean solve(int npieces, ArrayList<Move> ans) {
		if (npieces == 1) {
			return true;
		}
		for(int r=1; r<board.length-1; r++) {
			for(int c=1; c<board.length-1; c++)
				if (board[r][c] != ' ') {
					ArrayList<MoveTo> moves = getMoves(board[r][c], r, c);
					for(MoveTo m : moves) {
						char savePiece = board[m.row][m.col];
						char movedPiece = board[r][c];
						board[m.row][m.col] = movedPiece;
						board[r][c] = ' ';
						if (solve(npieces-1, ans)) {
							ans.add(0, new Move(movedPiece, m.row, m.col, r, c));
							return true;
						}
						board[r][c] = movedPiece;
						board[m.row][m.col] = savePiece;
					}
				}
		}
		return false;
	}

	public static ArrayList<MoveTo> getMoves(char piece, int row, int col)
	{
		ArrayList<MoveTo> moves = new ArrayList<>();
		int size = board.length;
		switch(piece) {
		case 'P' :
			if(board[row+1][col-1] != ' ')
				moves.add(new MoveTo(row+1, col-1));
			if(board[row+1][col+1] != ' ')
				moves.add(new MoveTo(row+1, col+1));
			break;
		case 'K' :
			for(int i=-1; i<=1; i++)
				for(int j=-1; j<=1; j++) {
					if (i == 0 && j == 0)
						continue;
					if (board[row+i][col+j] != ' ')
						insert(moves, new MoveTo(row+i, col+j));
				}
			break;
		case 'N':
			if (row > 2) {
				for(int j=-1; j<=1; j+=2)
					if(board[row-2][col+j] != ' ')
						insert(moves, new MoveTo(row-2, col+j));	
			}
			if (row < size - 2) {
				for(int j=-1; j<=1; j+=2)
					if(board[row+2][col+j] != ' ')
						insert(moves, new MoveTo(row+2, col+j));
			}
			if (col > 2) {
				for(int i=-1; i<=1; i+=2)
					if(board[row+i][col-2] != ' ')
						insert(moves, new MoveTo(row+i, col-2));	
			}
			if (col < size - 2) {
				for(int i=-1; i<=1; i+=2)
					if(board[row+i][col+2] != ' ')
						insert(moves, new MoveTo(row+i, col+2));	
			}
			break;
		case 'Q':
		case 'B':
			int r=row+1, c = col+1;
			while(r < size-1 && c < size) {
				if(board[r][c] != ' ') {
					insert(moves, new MoveTo(r, c));
					break;
				}
				r++; c++;
			}
			r=row+1; c = col-1;
			while(r < size-1 && c > 0) {
				if(board[r][c] != ' ') {
					insert(moves, new MoveTo(r, c));
					break;
				}
				r++; c--;
			}
			r=row-1; c = col-1;
			while(r > 0 && c > 0) {
				if(board[r][c] != ' ') {
					insert(moves, new MoveTo(r, c));
					break;
				}
				r--; c--;
			}
			r=row-1; c = col+1;
			while(r > 0 && c < size-1) {
				if(board[r][c] != ' ') {
					insert(moves, new MoveTo(r, c));
					break;
				}
				r--; c++;
			}
			if (piece == 'B')
				break;
		case 'R':
			for(r=row+1; r<size-1; r++) {
				if(board[r][col] != ' ') {
					insert(moves, new MoveTo(r, col));
					break;
				}
			}
			for(r=row-1; r>0; r--) {
				if(board[r][col] != ' ') {
					insert(moves, new MoveTo(r, col));
					break;
				}
			}
			for(c=col+1; c<size-1; c++) {
				if(board[row][c] != ' ') {
					insert(moves, new MoveTo(row, c));
					break;
				}
			}
			for(c=col-1; c>0; c--) {
				if(board[row][c] != ' ') {
					insert(moves, new MoveTo(row, c));
					break;
				}
			}
			break;
		}
		return moves;
	}

	public static void insert(ArrayList<MoveTo> list, MoveTo m)
	{
		int i=0;
		while (i < list.size()) {
			MoveTo m1 = list.get(i);
			if (m1.row > m.row)
				break;
			if (m1.row == m.row && m1.col > m.col)
				break;
			i++;
		}
		list.add(i,m);
	}

	public static String convert(int r, int c)
	{
		return "" + (char)('A'+r-1) + c;
	}
}
