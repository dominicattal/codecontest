import java.util.Scanner;
import java.util.Arrays;

public class BaseLoHi_JPB {

	public static String digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvxyz";
	
	public static void main(String[] args) {
		Scanner in = new Scanner(System.in);
		int base = in.nextInt();
		int ntest = in.nextInt();
		
		for(int itest = 1; itest <= ntest; itest++) {
			int ndigits = in.nextInt();
			int [] low = new int[ndigits];
			int [] high = new int[ndigits];
			int [] iguess = new int[ndigits];
			Arrays.fill(low, 0);
			Arrays.fill(high, base-1);
			boolean cheater = false;
			while (!cheater) {
				for(int i=0; i<ndigits; i++)
					iguess[i] = (low[i] + high[i])/2;
				System.out.println(buildStringGuess(iguess));
				System.out.flush();
				String response = in.next();
				if (response.equals("correct"))
					break;
				else {
					int neq = 0;
					for(int i=0; i<ndigits; i++) {
						char ch = response.charAt(i);
						if (ch == '=') {
							low[i] = high[i] = iguess[i];
							neq++;
						}
						else if (ch == '-') {
							if (iguess[i] == low[i]) {
								System.out.println("cheater");
								System.out.flush();
								cheater = true;
								break;
							}
							else
								high[i] = iguess[i]-1;
						}
						else {	// ch == '+'
							if (iguess[i] == high[i]) {
								System.out.println("cheater");
								System.out.flush();
								cheater = true;
								break;
							}
							else
								low[i] = iguess[i]+1;
						}					
					}
					if (neq == ndigits) {				// all ='s should be bad, right?
						System.out.println("cheater");
						System.out.flush();
						cheater = true;
					}
				}
			}
			if(cheater){
				String response = in.next();
			}
		}
	}

	public static String buildStringGuess(int [] g)
	{
		String s = "";
		for(int i=0; i< g.length; i++)
			s += digits.charAt(g[i]);
		return s;
	}
}
