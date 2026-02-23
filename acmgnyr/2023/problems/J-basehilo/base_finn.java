/**
 * Use a binary search on each digit.
 *
 * Keep track of contradictory cases:
 * - Not giving '=' when '=' previously given or there is only one choice remaining.
 * - Giving '-' (i.e., too big) when we have already given the largest remaining valid value for the digit.
 * - Giving '+' (i.e., too small) when we have already given the smallest remaining valid value for the digit.
 * Number of guesses required: ceil(log_2(B)) + 1
 *
 * @author Finn Lidbetter
 */

import java.util.*;

public class base_finn {
    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        int base = sc.nextInt();
        int numCases = sc.nextInt();
        for (int test=0; test < numCases; test++) {
            int numDigits = sc.nextInt();
            sc.nextLine();
            StringBuilder sb = new StringBuilder();
            for (int i=0; i<numDigits; i++) {
                sb.append("=");
            }
            String allEqualStr = sb.toString();
            int[] lo = new int[numDigits];
            int[] hi = new int[numDigits];
            int[] mid = new int[numDigits];
            boolean[] locked = new boolean[numDigits];
            Arrays.fill(lo, 0);
            Arrays.fill(hi, base - 1);
            boolean done = false;
            while (!done) {
                sb = new StringBuilder();
                for (int i=0; i<numDigits; i++) {
                    if (locked[i]) {
                        mid[i] = mid[i];
                    } else {
                        mid[i] = lo[i] + ((hi[i] - lo[i]) / 2);
                    }
                    if (mid[i] < 10) {
                        sb.append(mid[i]);
                    } else if (mid[i] < 36) {
                        sb.append((char)('A' + (mid[i] - 10)));
                    } else {
                        sb.append((char)('a' + (mid[i] - 36)));
                    }
                }
                System.out.println(sb.toString());
                System.out.flush();
                String response = sc.nextLine();
                if (response.equals("correct")) {
                    done = true;
                    break;
                } else {
                    if (response.equals(allEqualStr)) {
                        System.out.println("cheater");
                        System.out.flush();
		        // Read the "correct" response
		        sc.nextLine();
                        done = true;
                        continue;
                    }
                    char[] responseArr = response.toCharArray();
                    for (int i=0; i<numDigits; i++) {
                        boolean cheating = false;
                        if ((locked[i] || lo[i] == hi[i]) && responseArr[i] != '=') {
                            cheating = true;
                        }
                        if (mid[i] == lo[i] && responseArr[i] == '-') {
                            cheating = true;
                        }
                        if (mid[i] == hi[i] && responseArr[i] == '+') {
                            cheating = true;
                        }
                        if (cheating) {
                            System.out.println("cheater");
                            System.out.flush();
                            // Read the "correct" response
                            sc.nextLine();
                            done = true;
                            break;
                        }
                        if (responseArr[i] == '=') {
                            lo[i] = mid[i];
                            hi[i] = mid[i];
                            locked[i] = true;
                        } else if (responseArr[i] == '+') {
                            // Too small.
                            lo[i] = mid[i] + 1;
                        } else {
                            // Too big.
                            hi[i] = mid[i] - 1;
                        }
                    }
                }
            }
        }
    }
}
