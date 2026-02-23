// Liam Keliher, 2023
// Solution for problem "ISBN Conversion" (isbnconversion)


import java.io.*;

public class ISBNConversionKeliher {
    //---------------------------------------------------------------
    public static void main(String[] args) throws IOException {
        BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
        int numCases = Integer.parseInt(br.readLine());
        StringBuilder sb = new StringBuilder(20*numCases);
        for (int c = 0; c < numCases; c++) {
            String isbn = br.readLine();
            if (!isValid(isbn)) {
                sb.append("invalid\n");
            } // if
            else {
                sb.append(convert(isbn)).append('\n');
            } // else
        } // for c
        System.out.print(sb);
    } // main(String[])
    //---------------------------------------------------------------
    static boolean isValid(String isbn) {
        int len = isbn.length();

        // Count the number of hyphens
        int numHyphens = 0;
        for (int i = 0; i < len; i++) {
            char ch = isbn.charAt(i);
            if (ch == '-') {
                numHyphens++;
            } // if
        } // if

        // Check that there are exactly 10 digits
        int numDigits = len - numHyphens;
        if (numDigits != 10) {
            return false;
        } // if

        // Check that if 'X' appears, it is only in the last position
        int xPos = isbn.indexOf('X');
        if (xPos != -1 && xPos != len - 1) {
            return false;
        } // if

        // Check that the first and last digits are non-hyphens
        if (isbn.charAt(0) == '-' || isbn.charAt(len - 1) == '-') {
            return false;
        } // if

        // Check that "--" is not a substring
        if (isbn.indexOf("--") != -1) {
            return false;
        } // if

        // Check that if there are 3 hyphens, one of them is immediately before the checksum digit
        if (numHyphens == 3 && isbn.charAt(len - 2) != '-') {
            return false;
        } // if

        // Apply the checksum formula
        int sum = 0;
        int coeff = 10;
        for (int i = 0; i < len; i++) {
            char ch = isbn.charAt(i);
            if (ch != '-') {
                int digitVal = 10;   // if digit = X
                if ('0' <= ch && ch <= '9') {
                    digitVal = ch - '0';
                } // if
                sum += coeff * digitVal;
                coeff--;
            } // if
        } // for i
        if (sum % 11 != 0) {
            return false;
        } // if

        return true;
    } // isValid(String)
    //---------------------------------------------------------------
    static String convert(String isbn) {
        isbn = "978-" + isbn.substring(0, isbn.length() - 1);
        int sum = 0;
        int coeff = 1;
        int len = isbn.length();
        for (int i = 0; i < len; i++) {
            char ch = isbn.charAt(i);
            if (ch != '-') {
                sum += coeff * (ch - '0');
                coeff = 4 - coeff;
            } // if
        } // for i
        int checksumDigit = (10 - sum % 10) % 10;
        return isbn + checksumDigit;
    } // convert(String)
    //---------------------------------------------------------------
} // class ISBNConversionKeliher

