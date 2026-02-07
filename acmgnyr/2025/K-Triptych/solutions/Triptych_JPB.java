import java.util.Scanner;

public class Triptych_JPB {

	public static long[][][][][] table = new long[25][25][25][3][3]; // numa, numb, numc, 2nd last char, last char
	public static int w, d, max, min;

	public static void main(String[] args) {
		Scanner in = new Scanner(System.in);
		w = in.nextInt();
		d = in.nextInt();
		max = Math.min(w, (w+2*d)/3);
		min = Math.max(0, (w-2*d+2)/3);
		table[1][1][0][0][1] = 1;		// "ab"
		table[1][1][0][1][0] = 1;		// "ba"
		table[1][0][1][0][2] = 1;		// "ac"
		table[1][0][1][2][0] = 1;		// "ca"
		table[0][1][1][1][2] = 1;		// "bc"
		table[0][1][1][2][1] = 1;		// "cb"
		table[0][2][0][1][1] = 1;		// "bb"
		for(int week = 3; week <= w; week++) {
			for(int na = 0; na<=max; na++) {
				for(int nb = 0; nb<=max; nb++) {		// can do better here
					int nc = week - na - nb;
					if (nc < 0 || nc > max)
						continue;
					if (na > 0) {
						table[na][nb][nc][1][0] = table[na-1][nb][nc][0][1] + table[na-1][nb][nc][1][1] + table[na-1][nb][nc][2][1];
						table[na][nb][nc][2][0] = table[na-1][nb][nc][0][2] + table[na-1][nb][nc][1][2];
					}
														// strings that end with 'b'
					if (nb > 0) {
						table[na][nb][nc][0][1] = table[na][nb-1][nc][1][0] + table[na][nb-1][nc][2][0];
						table[na][nb][nc][1][1] = table[na][nb-1][nc][0][1] + table[na][nb-1][nc][2][1];
						table[na][nb][nc][2][1] = table[na][nb-1][nc][0][2] + table[na][nb-1][nc][1][2];
					}
														// strings that end with 'c' - symmetric with 'a'
					if (nc > 0) {
						table[na][nb][nc][0][2] = table[na][nb][nc-1][1][0] + table[na][nb][nc-1][2][0];
						table[na][nb][nc][1][2] = table[na][nb][nc-1][0][1] + table[na][nb][nc-1][1][1] + table[na][nb][nc-1][2][1];
					}
				}
			}
		}

		long count = 0;
		for(int na = min; na<=max; na++) {
			for(int nb=min; nb<=max; nb++) {
				int nc = w - na - nb;
				if (goodNums(na, nb, nc)) {
					for(int i=0; i<3; i++)
						for(int j=0; j<3; j++)
							count += table[na][nb][nc][i][j];
				}
			}
		}
		long palins = 0;
		if (w == 2 && d >= 2)				// two special cases
			palins = 1;
		else if (w == 3 && d >= 2)
			palins = 6;
		else if (w%2 == 0) {
			for(int na = (min+1)/2; na <= max/2; na++) {
				for(int nb = (min+1)/2; nb <= max/2; nb++) {
					int nc = w/2-na-nb;
					if (goodNums(2*na, 2*nb, 2*nc))
						palins += table[na][nb][nc][0][1] + table[na][nb][nc][2][1];
				}
			}
		}
		else {
			for(int na = min/2; na <= max/2; na++) {
				for(int nb = min/2; nb <= max/2; nb++) {
					int nc = w/2-na-nb;
					if (2*nc >= min && 2*nc <= max) {
												// palins with 'a' in the center
						long acount = 0;
						if (goodNums(2*na+1, 2*nb, 2*nc))
							acount += table[na][nb][nc][1][1] + table[na][nb][nc][1][2] + table[na][nb][nc][2][1]
										+table[na][nb][nc][0][1] + table[na][nb][nc][0][2];
												// palins with 'b' in the center
						if (goodNums(2*na, 2*nb+1, 2*nc))
							palins += table[na][nb][nc][0][2] + table[na][nb][nc][2][0]
									+ table[na][nb][nc][1][0] + table[na][nb][nc][1][2];
												// palins with 'c' in the center - same as with a
						palins += 2*acount;
					}
				}
			}		
		}
		System.out.println(count - palins);
/*
		for(int na = 0; na<=max; na++) {
			for(int nb=0; nb<=max; nb++) {
				for(int nc = 0; nc<=max; nc++) {
					for(char c1 = 'a'; c1 <= 'c'; c1 = (char)(c1+1)) {
						for(char c2 = 'a'; c2 <= 'c'; c2 = (char)(c2+1)) {
							System.out.println(na + "," + nb + "," + nc + "," + c1 + "," + c2 + ": " + table[na][nb][nc][c1-'a'][c2-'a']);
						}
					}
				}
			}
		}
/**/
	}

	public static boolean goodNums(int na, int nb, int nc) {
		return (nc >= 0 && Math.abs(na-nb) <= d && Math.abs(nb-nc) <= d && Math.abs(nc-na) <= d);
	}
}
