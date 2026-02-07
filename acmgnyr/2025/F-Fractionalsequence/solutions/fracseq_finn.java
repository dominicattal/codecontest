import java.util.*;
import java.io.*;


public class fracseq_finn {
    static long gcd(long a, long b) {
        return b == 0 ? a : gcd(b, a%b);
    }

    public static void main(String[] args) throws IOException {
        BufferedReader br = new BufferedReader(new InputStreamReader(System.in));

        long n = Long.parseLong(br.readLine());
        // Get the least integer, k, such that k*(k+1)/2 >= n
        /*
        k(k+1)/2 = n;
        k*k/2 + k/2 - n = 0;
        k = ceil(-1/2 + sqrt(1/4+2*n));
         */

        long k = (long) Math.ceil((Math.sqrt(2 * n + 0.25) - 0.5));
        long f = (n - k * (k - 1) / 2) - 1;
        if (f == 0) {
            System.out.println(k);
        } else {
            System.out.printf("%d %d/%d\n", k, f/gcd(f,k), k/gcd(f,k));
        }
    }
}
