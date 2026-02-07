import java.util.Scanner;

public class HowManyBalls_Bon {

	public static void main(String[] args) {
		Scanner in = new Scanner(System.in);
		long p = in.nextLong();
		long q = in.nextLong();
		for(long r=1; r<1000000; r++) {
			long gl = r;
			long gu = 9999999;
			if (2*r*gu*q > (r+gu)*(r+gu-1)*p) {
				continue;
			}
			long g=0;
			while (gu - gl > 1) {
				g = (gu+gl)/2;
				if (2*r*g*q > (r+g)*(r+g-1)*p) {
					gl = g;
				}
				else {
					gu = g;
				}
			}
			g--;
			if (2*r*g*q == (r+g)*(r+g-1)*p) {
				System.out.println(r + " " + g);
				System.exit(0);
			}
			g++;
			if (2*r*g*q == (r+g)*(r+g-1)*p) {
				System.out.println(r + " " + g);
				System.exit(0);
			}
			g++;
			if (2*r*g*q == (r+g)*(r+g-1)*p) {
				System.out.println(r + " " + g);
				System.exit(0);
			}
		}
		System.out.println("impossible");
	}
}

