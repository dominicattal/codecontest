import java.util.Scanner;

public class MorseCodes_JPB {

	public static char[] chars = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
								  'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
								  'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0',
								  '1', '2', '3', '4', '5', '6', '7', '8', '9'};

	public static String[] codes = {"._",   "_...", "_._.", "_..",  ".",    ".._.", "__.",  "....", "..",
			                        ".___", "_._",  "._..", "__",   "_.",   "___",  ".__.", "__._", "._.",
			                        "...",  "_",    ".._",  "..._", ".__",  "_.._", "_.__", "__..", "_____",
			                        ".____","..___","...__","...._",".....","_....","__...","___..","____."};

	public static String reverse(String s)
	{
		String rs = ""+s.charAt(0);
		for(int j=1; j<s.length(); j++)
			rs = s.charAt(j) + rs;
		return rs;
		
	}
	
	public static boolean isPalin(String s)
	{
		int n = s.length();
		for(int i=0; i<n/2; i++) {
			if (s.charAt(i) != s.charAt(n-i-1))
				return false;
		}
		return true;
	}
	
	public static void main(String[] args) {

		Scanner in = new Scanner(System.in);
		String word = in.nextLine();
		String s = "";
		for(int i=0; i<word.length(); i++) {
			char ch = word.charAt(i);
			if (Character.isAlphabetic(ch))
				s += codes[ch-'A'];
			else if (Character.isDigit(ch))
				s += codes[ch-'0'+26];
		}

		if (isPalin(s))
			System.out.println(0);
		else {
			for(int i=1; i<s.length(); i++) {
				if (!isPalin(s.substring(i)))
					continue;
				findMatch3(reverse(s.substring(0,i)));
				break;
			}
		}
	}
	
	public static void findMatch3(String s)
	{
		int n = s.length();
		int [] len = new int[n+1];
		String [] matches = new String [n+1];
		
		len[0] = 0;
		matches[0] = "";
		for(int i=0; i<n; i++) {
			int bestlen = len[i]+1;
			String bestMatch = matches[i] + 'T';
			if (s.charAt(i) == '.')
				bestMatch = matches[i] + 'E';
			for(int j=2; j<=5; j++) {
				if (j > i+1)
					continue;
				char ch = findCharMatch(s.substring(i-j+1, i+1));
				if (ch != ' ' && len[i-j+1]+1 < bestlen) {
					bestlen = len[i-j+1]+1;
					bestMatch = matches[i-j+1]+ch;
				}
			}
			matches[i+1] = bestMatch;
			len[i+1] = bestlen;
		}
		System.out.println(len[n] + " " + matches[n]);
	}
	
	public static char findCharMatch(String s) {
		for(int i=0; i<codes.length; i++) {
			if (s.equals(codes[i]))
				return chars[i];
		}
		return ' ';
	}
}
