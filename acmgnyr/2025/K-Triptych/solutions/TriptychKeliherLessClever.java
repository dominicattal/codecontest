// Liam Keliher, 2023
//
// Solution for problem "Triptych" (triptych)
//
// Similar to TriptychKeliher.java, but is less clever when generating
// triples (a,b,c) / (aHalf,bHalf,cHalf), i.e., only uses the constraints
// a + b + c <= W / aHalf + bHalf + cHalf <= wHalf, not anything involving
// D and dHalf. The result is that more triples are generated and discarded
// (but there is no noticeable effect on performance).


import java.io.*;
import java.util.*;

public class TriptychKeliherLessClever {
    static final int A = 0;
    static final int B1 = 1;
    static final int B2 = 2;
    static final int C = 3;
    static long[][][][] memo;
    static final int EMPTY = -1;
	//---------------------------------------------------------------
	public static void main(String[] args) throws IOException {
        BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
        String[] tokens = br.readLine().split(" ");
        int W = Integer.parseInt(tokens[0]);
        int D = Integer.parseInt(tokens[1]);

        // Initialize memo table
        memo = new long[W + 1][W + 1][W + 1][4];
        for (int a = 0; a <= W; a++) {
            for (int b = 0; b <= W; b++) {
                for (int c = 0; c <= W; c++) {
                    Arrays.fill(memo[a][b][c], EMPTY);
                } // for c
            } // for b
        } // for a

        long answer = 0;
        for (int a = 0; a <= W; a++) {
            for (int b = 0; b <= W - a; b++) {
                int c = W - a - b;
                if (Math.abs(a - b) <= D && Math.abs(a - c) <= D && Math.abs(b - c) <= D) {
                    answer += recurse(a, b, c, A) + recurse(a, b, c, B1) + recurse(a, b, c, B2) + recurse(a, b, c, C);
                } // if
            } // for b
        } // for a

        // Remove palindromes that have already been counted:
        // - odd length
        // - even length with BB in the middle
        long numPalindromes = 0;
        int wHalf = W/2;
        for (int aHalf = 0; aHalf <= wHalf; aHalf++) {
            for (int bHalf = 0; bHalf <= wHalf - aHalf; bHalf++) {
                int cHalf = wHalf - aHalf - bHalf;
                int a = 2*aHalf;
                int b = 2*bHalf;
                int c = 2*cHalf;

                if (W % 2 == 0) {   // W is even
                    if (Math.abs(a - b) <= D && Math.abs(a - c) <= D && Math.abs(b - c) <= D) {
                        numPalindromes += recurse(aHalf, bHalf, cHalf, B1);
                    } // if
                } // if
                else {   // W is odd
                    // Put A in middle position
                    a++;
                    if (Math.abs(a - c) <= D && Math.abs(a - b) <= D && Math.abs(b - c) <= D) {
                        numPalindromes += recurse(aHalf + 1, bHalf, cHalf, A);
                    } // if
                    a--;

                    // Put B in middle position
                    b++;
                    if (Math.abs(a - c) <= D && Math.abs(a - b) <= D && Math.abs(b - c) <= D) {
                        numPalindromes += recurse(aHalf, bHalf + 1, cHalf, B1);   // cannot be B2
                    } // if
                    b--;

                    // Put C in middle position
                    c++;
                    if (Math.abs(a - c) <= D && Math.abs(a - b) <= D && Math.abs(b - c) <= D) {
                        numPalindromes += recurse(aHalf, bHalf, cHalf + 1, C);
                    } // if
                    c--;
                } // else
            } // for bHalf
        } // for aHalf
        answer -= numPalindromes;
        System.out.println(answer);
    } // main(String[])
	//---------------------------------------------------------------
    // Return the number of sequences containing numA A's, numB B's, and numC C's
    // in which there are no repeated consecutive letters, except perhaps BB, and
    // which end with A, B, BB, or C, as specified by last.
    static long recurse(int numA, int numB, int numC, int last) {
        if (memo[numA][numB][numC][last] != EMPTY) {
            return memo[numA][numB][numC][last];
        } // if

        long result = 0;
        int total = numA + numB + numC;
        // NOTE: it will never be the case that total == 0
        if (total == 1) {
            if ((last == A && numA == 1) || (last == B1 && numB == 1) || (last == C && numC == 1)) {
                result = 1;
            } // if
        } // if
        else {   // total >= 2
            if (last == A && numA > 0) {
                result = recurse(numA - 1, numB, numC, B1) + recurse(numA - 1, numB, numC, B2) + recurse(numA - 1, numB, numC, C);
            } // if
            else if (last == B1 && numB > 0) {
                result = recurse(numA, numB - 1, numC, A) + recurse(numA, numB - 1, numC, C);
            } // else if
            else if (last == B2 && numB > 0) {
                result = recurse(numA, numB - 1, numC, B1);
            } // else if
            else if (last == C && numC > 0) {
                result = recurse(numA, numB, numC - 1, A) + recurse(numA, numB, numC - 1, B1) + recurse(numA, numB, numC - 1, B2);
            } // else if
        } // else

        memo[numA][numB][numC][last] = result;
        return result;
    } // recurse(int,int,int,int)
	//---------------------------------------------------------------
} // class TriptychKeliherLessClever

