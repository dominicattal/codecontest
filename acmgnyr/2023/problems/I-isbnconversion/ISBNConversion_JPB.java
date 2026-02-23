import java.util.Scanner;

public class ISBNConversion_JPB {

	public static void main(String[] args) {
		Scanner in = new Scanner(System.in);

		int t = in.nextInt();
		for(int i=1; i<=t; i++) {
			String isbn10 = in.next();
			if (!ok(isbn10)) {
				System.out.println("invalid");
			}
			else {
				String isbn13 = "978-" + isbn10.substring(0, isbn10.length()-1);
				isbn13 += checkDigit13(isbn13);
				System.out.println(isbn13);
			}
		}
		in.close();
	}

	public static boolean ok(String s)
	{
		int len = s.length();
												// check for hyphens
		int count = 0;
		boolean prev = false;
		for(int i=0; i<len; i++) {
			if (s.charAt(i) == '-') {
				if (prev)
					return false;				// two hyphens in a row
				if(i==0 || i == len-1)
					return false;				// hyphen at start or end
				count++;
				prev = true;
			}
			else
				prev = false;
		}
		if (count > 3)
			return false;
		if (count == 3 && s.charAt(len-2) != '-')
			return false;
												// calculate check digit
		int check = 0, factor = 10;
		for(int i=0; i<len-1; i++) {
			int val = 0;
			try {
				val = Integer.parseInt(s.charAt(i)+"");
				check += val*factor;
				factor--;
			}
			catch (NumberFormatException e) {
				if (s.charAt(i) != '-')
					return false;
			}
		}
		if (factor != 1)
			return false;						// not 9 digits
		check = (11 - (check%11))%11;
		int last = 0;
		try {
			last = Integer.parseInt(s.charAt(len-1)+"");
		}
		catch (NumberFormatException e) {
			if (s.charAt(len-1) != 'X')
				return false;
			last = 10;
		}
		return (check == last);
	}

	public static int checkDigit13(String s)
	{
		int check = 0, factor = 1;
		int len = s.length();
		for(int i=0; i<len; i++) {
			char ch = s.charAt(i);
			if(ch != '-') {
				int val = Integer.parseInt(ch+"");
				check += val*factor;
				factor = 4-factor;;
			}
		}

		return (10 - (check%10))%10;
	}

}
