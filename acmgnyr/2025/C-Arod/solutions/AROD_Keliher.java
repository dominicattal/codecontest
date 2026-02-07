// Liam Keliher, 2025
//
// Solution for problem "AROD" (arod)
//
// Two main optimizations:
// (1) Consider two triangles that are horizontal and/or vertical translations
// of each other to be equivalent, and only generate one representative for each
// equivalence class, namely the member that is "jammed into" the bottom-left corner,
// and then compute the number of translations of this representative (i.e., the size
// of the equivalence class).
//
// (2) Use closed forms wherever possible to eliminate loops.
//
// Complexity: O(n^3), where n = max(X,Y)


import java.io.*;

public class AROD_Keliher {
    static final int NUM_TYPES = 4;
    static final int ACUTE = 0;
    static final int RIGHT = 1;
    static final int OBTUSE = 2;
    static final int DEGENERATE = 3;
	//---------------------------------------------------------------
	public static void main(String[] args) throws IOException {
        BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
        String[] tokens = br.readLine().split(" ");
        long xMax = Integer.parseInt(tokens[0]);
        long yMax = Integer.parseInt(tokens[1]);

        long[] counter = new long[NUM_TYPES];

        //***************************************
        // CASE 1: one vertex is on the left edge
        // Complexity: O(n^3)
        //***************************************
        case1(xMax, yMax, counter);

        //******************************************
        // CASE 2: two vertices are on the left edge
        // Complexity: O(n^2)
        //******************************************
        case2(xMax, yMax, counter);

        //********************************************
        // CASE 3: three vertices are on the left edge
        // Complexity: O(1)
        //********************************************
        case3(xMax, yMax, counter);

        // Print the results
        StringBuilder sb = new StringBuilder(100);
        for (int t = 0; t < NUM_TYPES; t++) {
            sb.append(counter[t]).append('\n');
        } // for t
        System.out.print(sb);
    } // main(String[])
	//---------------------------------------------------------------
    // CASE 1: one vertex is on the left edge
    // Complexity: O(n^3)
    static void case1(long xMax, long yMax, long[] counter) {
        //****************************************************************************
        // Subcase 1-A: the two vertices not on the left edge are horizontally aligned
        // Complexity: O(1)
        //****************************************************************************
        // Subsubcase 1-A-1: yLeft = yOtherLeft1 = yOtherRight2 = 0 (degenerate)
        // - counterpart of Case 3
        // Complexity: O(1)
        counter[DEGENERATE] += (xMax - 1)*xMax*(xMax + 1)*(yMax + 1)/6;

        // Subsubcase 1-A-2: yLeft = 0, yOtherLeft1 = yOtherRight2 > 0 (obtuse triangle)
        // - counterpart of Subcase 2-B
        // Complexity: O(1)
        long subsubcase1A2 = (xMax - 1)*xMax*(xMax + 1)*yMax*(yMax + 1)/12;
        counter[OBTUSE] += subsubcase1A2;

        // Subsubcase 1-A-2-flip: yLeft > 0, yOtherLeft1 = yOtherRight2 = 0 (obtuse triangle)
        // - mirror image of Subsubcase 1-A-2 (flip top to bottom)
        // Complexity: O(1)
        counter[OBTUSE] += subsubcase1A2;


        //**************************************************************************
        // Subcase 1-B: the two vertices not on the left edge are vertically aligned
        // - mirror image of Case 2 (flip left to right)
        // Complexity: O(n^2)
        //**************************************************************************
        case2(xMax, yMax, counter);


        //***********************************************************************************
        // Subcase 1-C: line through the two vertices not on the left edge has positive slope
        // Complexity: O(n^3)
        //***********************************************************************************
        // Subsubcase 1-C-1: yLeft = 0, yOtherLeft = 0 (obtuse triangle)
        // - counterpart of Subsubcase 1-A-2
        // Complexity: O(1)
        counter[OBTUSE] += subsubcase1A2;

        // Subsubcase 1-C-2: yLeft = 0, yOtherLeft > 0 (obtuse triangle or degenerate)
        // Complexity: O(n^2)
        subsubcase1C2(xMax, yMax, counter);

        // Subsubcase 1-C-3: yLeft > 0, yOtherLeft = 0
        // Complexity: O(n^3)
        subsubcase1C3(xMax, yMax, counter);


        //***********************************************************************************
        // Subcase 1-D: line through the two vertices not on the left edge has negative slope
        //***********************************************************************************
        // Subsubcase 1-D-1: yLeft = 0, yOtherRight = 0
        // - counterpart of Subcase 2-C (with xMax and yMax interchanged)
        // Complexity: O(n^2)
        subcase2C(yMax, xMax, counter);

        // Subsubcase 1-D-2: yLeft = 0, yOtherRight > 0
        // - counterpart of Subsubcase 1-C-3, Region 1 (flip top to bottom)
        // Complexity: O(n^3)
        for (int yLeft = 1; yLeft <= yMax; yLeft++) {   // start at 1
            for (int xOL = 1; xOL <= xMax; xOL++) {   // start at 1, stop before xMax
                subsubcase1C3_Region1(xMax, yMax, counter, yLeft, xOL);
            } // for xOL
        } // for yLeft

        // Subsubcase 1-D-3: yLeft > 0, yOtherRight = 0
        // - counterpart of Subsubcase 1-A-2 + Subsubcase 1-C-2 + Subsubcase 1-D-2 (flip left to right)
        // Complexity: O(n^3)
        counter[OBTUSE] += subsubcase1A2;
        subsubcase1C2(xMax, yMax, counter);
        // Run code for Subsubcase 1-D-2 again
        for (int yLeft = 1; yLeft <= yMax; yLeft++) {   // start at 1
            for (int xOL = 1; xOL <= xMax; xOL++) {   // start at 1, stop before xMax
                subsubcase1C3_Region1(xMax, yMax, counter, yLeft, xOL);
            } // for xOL
        } // for yLeft
    } // case1(long,long,long[])
	//---------------------------------------------------------------
    // Subsubcase 1-C-2: yLeft = 0, yOtherLeft > 0 (obtuse triangle or degenerate)
    // Complexity: O(n^2)
    static void subsubcase1C2(long xMax, long yMax, long[] counter) {
        // xOL = xOtherLeft, yOL = yOtherLeft, xOR = xOtherRight, yOR = yOtherRight
        for (int xOL = 1; xOL < xMax; xOL++) {   // start at 1, stop before xMax
            for (int yOL = 1; yOL < yMax; yOL++) {   // start at 1, stop before yMax
                // *** INITIALLY IGNORE DEGNERATE TRIANGLES (ASSUME EVERYTHING IS OBTUSE) ***
                // Answer = \sum_{xOR=xOL+1}^xMax \sum_{yOR=yOL+1}^{yMax} (xMax-xOR+1)(yMax-yOR+1)
                //    (xMax-xOR+1) is the number of horizontal translations
                //    (yMax-yOR+1) is the number of vertical translations
                // Closed form: (1/4)(xMax*xMax+xMax-2*xMax*xOL+xOL*xOL-xOL)*(yMax*yMax+yMax-2*yMax*yOL+yOL*yOL-yOL)
                long numObtuse = (xMax*xMax + xMax - 2*xMax*xOL + xOL*xOL - xOL)*(yMax*yMax + yMax - 2*yMax*yOL + yOL*yOL - yOL)/4;

                // *** COUNT THE DEGENERATE TRIANGLES ***
                int[] slope = getSlope(0, 0, xOL, yOL);
                int rise = slope[0];
                int run = slope[1];
                int N = (int)Math.min((xMax - xOL)/run, (yMax - yOL)/rise);
                // Answer = \sum_{k=1}^N (xMax-(xOL+k*run)+1)(yMax-(yOL+k*rise)+1)
                //    (xMax-(xOL+k*run)+1) is the number of horizontal translations
                //    (yMax-(yOL+k*rise)+1) is the number of vertical translations
                // Closed form: (1/6)(N)(6*xTerm*yTerm-3(xTerm*rise+yTerm*run)(N+1)+rise*run*N*(2*N+1))
                long xTerm = xMax - xOL + 1;
                long yTerm = yMax - yOL + 1;
                long numDegenerate = N*(6*xTerm*yTerm - 3*(xTerm*rise + yTerm*run)*(N+1) + rise*run*(N+1)*(2*N + 1))/6;
                counter[DEGENERATE] += numDegenerate;

                // *** UNDO OVERCOUNTING OF OBTUSE TRIANGLES ***
                numObtuse -= numDegenerate;
                counter[OBTUSE] += numObtuse;
            } // for yOL
        } // for xOL
    } // subsubcase1C2(long,long,long[])
    //---------------------------------------------------------------
    // Subsubcase 1-C-3: yLeft > 0, yOtherLeft = 0
    // Complexity: O(n^3)
    static void subsubcase1C3(long xMax, long yMax, long[] counter) {
        // xOL = xOtherLeft, yOR = yOtherRight
        for (int yLeft = 1; yLeft <= yMax; yLeft++) {   // start at 1
            for (int xOL = 1; xOL < xMax; xOL++) {   // start at 1, stop before xMax

                // Region 1 (ORARO): 0 < yOR < yLeft
                subsubcase1C3_Region1(xMax, yMax, counter, yLeft, xOL);

                int yMaxRegion2 = (xOL*xOL + yLeft*yLeft)/yLeft;
                yMaxRegion2 = (int)Math.min(yMaxRegion2, yMax);

                // Region 2 (ARO): yLeft <= yOR <= yMaxRegion2
                subsubcase1C3_Region2(xMax, yMax, counter, yLeft, xOL, yMaxRegion2);

                // Region 3 (ORARO): yMaxRegion2 < yOR
                subsubcase1C3_Region3(xMax, yMax, counter, yLeft, xOL, yMaxRegion2);

            } // for xOL
        } // for yLeft
    } // subsubcase1C3(long,long,long[])
	//---------------------------------------------------------------
    // Subsubcase 1-C-3, Region 1 (ORARO): 0 < yOR < yLeft
    // Complexity: O(n)
    static void subsubcase1C3_Region1(long xMax, long yMax, long[] counter, int yLeft, int xOL) {
        for (int yOR = 1; yOR < yLeft; yOR++) {   // start at 1, stop before yLeft
            int RELOR;   // right end of left obtuse range (matching left end = xOL + 1)
            int LEAR;    // left end of acute range
            int REAR;    // right end of acute range
            int LEROR;   // left end of right obtuse range (matching right end = xMax)

            // Find x-coord of intersection with circle
            int discriminant = xOL*xOL - 4*yOR*(yOR - yLeft);    // simplified discriminant
            int sqrtDiscriminant = (int)Math.sqrt(discriminant);
            int threshold = (xOL + sqrtDiscriminant)/2;
            // Check if x-coord on circle is an integer
            if (sqrtDiscriminant*sqrtDiscriminant == discriminant && (xOL + sqrtDiscriminant) % 2 == 0) {
                // Right triangle
                int xOnCircle = threshold;
                if (xOnCircle <= xMax) {
                    // (xMax-xOnCircle+1) is the number of horizontal translations
                    // (yMax-yLeft+1) is the number of vertical translations
                    counter[RIGHT] += (xMax - xOnCircle + 1)*(yMax - yLeft + 1);
                } // if
                RELOR = threshold - 1;
            } // if
            else {
                // No right triangle
                RELOR = threshold;
            } // else
            LEAR = threshold + 1;

            // Find x-coord of intersection with lower perpendicular
            int numer = yOR*yLeft + xOL*xOL;
            int denom = xOL;
            threshold = numer/denom;
            // Check if x-coord on lower perpendicular is an integer
            if (numer % denom == 0) {
                // Right triangle
                int xOnLowerPerpendicular = threshold;
                if (xOnLowerPerpendicular <= xMax) {
                    // (xMax-xOnLowerPerpendicular+1) is the number of horizontal translations
                    // (yMax-yLeft+1) is the number of vertical translations
                    counter[RIGHT] += (xMax - xOnLowerPerpendicular + 1)*(yMax - yLeft + 1);
                } // if
                REAR = threshold - 1;
            } // if
            else {
                // No right triangle
                REAR = threshold;
            } // else
            LEROR = threshold + 1;

            // Left obtuse range
            RELOR = (int)Math.min(RELOR, xMax);
            // Answer = \sum_{xOR=xOL+1}^{RELOR} (xMax-xOR+1)(yMax-yLeft+1)
            //    (xMax-xOR+1) is the number of horizontal translations
            //    (yMax-yLeft+1) is the number of vertical translations
            // Closed form: (1/2)(2(xMax)(RELOR-xOL)+RELOR(1-RELOR)+xOL(xOL-1))(yMax-yLeft+1)
            counter[OBTUSE] += (2*xMax*(RELOR - xOL) + RELOR*(1 - RELOR) + xOL*(xOL - 1))*(yMax - yLeft + 1)/2;

            // Acute range
            if (LEAR <= xMax) {
                REAR = (int)Math.min(REAR, xMax);
                // Answer = \sum_{xOR=LEAR}^{REAR} (xMax-xOR+1)(yMax-yLeft+1)
                //    (xMax-xOR+1) is the number of horizontal translations
                //    (yMax-yLeft+1) is the number of vertical translations
                // Closed form: (1/2)(2(xMax+1)(REAR-LEAR+1)-REAR(REAR+1)+(LEAR-1)LEAR)(yMax-yLeft+1)
                counter[ACUTE] += (2*(xMax + 1)*(REAR - LEAR + 1) - REAR*(REAR + 1) + (LEAR - 1)*LEAR)*(yMax - yLeft + 1)/2;
            } // if

            // Right obtuse range
            if (LEROR <= xMax) {
                // Answer = \sum_{xOR=LEROR}^{xMax} (xMax-xOR+1)(yMax-yLeft+1)
                //    (xMax-xOR+1) is the number of horizontal translations
                //    (yMax-yLeft+1) is the number of vertical translations
                // Closed form: (1/2)(2(xMax+1)(xMax-LEROR+1)-xMax(xMax+1)+(LEROR-1)LEROR)(yMax-yLeft+1)
                counter[OBTUSE] += (2*(xMax + 1)*(xMax - LEROR + 1) - xMax*(xMax + 1) + (LEROR - 1)*LEROR)*(yMax - yLeft + 1)/2;
            } // if
        } // for yOR
    } // subsubcase1C3_Region1(long,long,long[],int,int)
	//---------------------------------------------------------------
    // Subsubcase 1-C-3, Region 2 (ARO): yLeft <= yOR <= yMaxRegion2
    // Complexity: O(n)
    static void subsubcase1C3_Region2(long xMax, long yMax, long[] counter, int yLeft, int xOL, int yMaxRegion2) {
        for (int yOR = yLeft; yOR <= yMaxRegion2; yOR++) {
            int REAR;    // right end of acute range (matching left end = xOL + 1)
            int LEOR;   // left end of obtuse range (matching right end = xMax)

            // Find x-coord of intersection with lower perpendicular
            int numer = yOR*yLeft + xOL*xOL;
            int denom = xOL;
            int threshold = numer/denom;
            // Check if x-coord on lower perpendicular is an integer
            if (numer % denom == 0) {
                // Right triangle
                int xOnLowerPerpendicular = threshold;
                if (xOnLowerPerpendicular <= xMax) {
                    // (xMax-xOnLowerPerpendicular+1) is the number of horizontal translations
                    // (yMax-yOR+1) is the number of vertical translations
                    counter[RIGHT] += (xMax - xOnLowerPerpendicular + 1)*(yMax - yOR + 1);
                } // if
                REAR = threshold - 1;
            } // if
            else {
                // No right triangle
                REAR = threshold;
            } // else
            LEOR = threshold + 1;

            // Acute range
            REAR = (int)Math.min(REAR, xMax);
            // Answer = \sum_{xOR=xOL+1}^{REAR} (xMax-xOR+1)(yMax-yOR+1)
            //    (xMax-xOR+1) is the number of horizontal translations
            //    (yMax-yOR+1) is the number of vertical translations
            // Closed form: (1/2)(2(xMax)(REAR-xOL)-REAR(REAR-1)+xOL(xOL-1))(yMax-yOR+1)
            counter[ACUTE] += (2*xMax*(REAR - xOL) - REAR*(REAR - 1) + xOL*(xOL - 1))*(yMax - yOR + 1)/2;

            // Obtuse range
            if (LEOR <= xMax) {
                // Answer = \sum_{xOR=LEOR}^{xMax} (xMax-xOR+1)(yMax-yOR+1)
                //    (xMax-xOR+1) is the number of horizontal translations
                //    (yMax-yOR+1) is the number of vertical translations
                // Closed form: (1/2)(2(xMax+1)(xMax-LEOR+1)-xMax(xMax+1)+(LEOR-1)LEOR)(yMax-yOR+1)
                counter[OBTUSE] += (2*(xMax + 1)*(xMax - LEOR + 1) - xMax*(xMax + 1) + (LEOR - 1)*LEOR)*(yMax - yOR + 1)/2;
            } // if
        } // for yOR
    } // subsubcase1C3_Region2(long,long,long[],int,int,int)
	//---------------------------------------------------------------
    // Subsubcase 1-C-3, Region 3 (ORARO): yMaxRegion2 < yOR
    // Complexity: O(n)
    static void subsubcase1C3_Region3(long xMax, long yMax, long[] counter, int yLeft, int xOL, int yMaxRegion2) {
        for (int yOR = yMaxRegion2 + 1; yOR <= yMax; yOR++) {   // start at yMaxRegion2+1
            int RELOR;   // right end of left obtuse range (matching left end = xOL + 1)
            int LEAR;    // left end of acute range
            int REAR;    // right end of acute range
            int LEROR;   // left end of right obtuse range (matching right end = xMax)

            // Find x-coord of intersection with lower perpendicular
            int numer = yOR*yLeft + xOL*xOL;
            int denom = xOL;
            int threshold = numer/denom;
            // Check if x-coord on lower perpendicular is an integer
            if (numer % denom == 0) {
                // Right triangle
                int xOnLowerPerpendicular = threshold;
                if (xOnLowerPerpendicular <= xMax) {
                    // (xMax-xOnLowerPerpendicular+1) is the number of horizontal translations
                    // (yMax-yOR+1) is the number of vertical translations
                    counter[RIGHT] += (xMax - xOnLowerPerpendicular + 1)*(yMax - yOR + 1);
                } // if
                REAR = threshold - 1;
            } // if
            else {
                // No right triangle
                REAR = threshold;
            } // else
            LEROR = threshold + 1;

            // Find x-coord of intersection with upper perpendicular
            numer = yOR*yLeft - yLeft*yLeft;
            denom = xOL;
            threshold = numer/denom;
            // Check if x-coord on upper perpendicular is an integer
            if (numer % denom == 0) {
                // Right triangle
                int xOnUpperPerpendicular = threshold;
                if (xOnUpperPerpendicular <= xMax) {
                    // (xMax-xOnUpperPerpendicular+1) is the number of horizontal translations
                    // (yMax-yOR+1) is the number of vertical translations
                    counter[RIGHT] += (xMax - xOnUpperPerpendicular + 1)*(yMax - yOR + 1);
                } // if
                RELOR = threshold - 1;
            } // if
            else {
                // No right triangle
                RELOR = threshold;
            } // else
            LEAR = threshold + 1;

            // Left obtuse range
            RELOR = (int)Math.min(RELOR, xMax);
            // Answer = \sum_{xOR=xOL+1}^{RELOR} (xMax-xOR+1)(yMax-yOR+1)
            //    (xMax-xOR+1) is the number of horizontal translations
            //    (yMax-yOR+1) is the number of vertical translations
            // Closed form: (1/2)(2(xMax)(RELOR-xOL)-RELOR(RELOR-1)+xOL(xOL-1))(yMax-yOR+1)
            counter[OBTUSE] += (2*xMax*(RELOR - xOL) - RELOR*(RELOR - 1) + xOL*(xOL - 1))*(yMax - yOR + 1)/2;

            // Acute range
            if (LEAR <= xMax) {
                REAR = (int)Math.min(REAR, xMax);
                // Answer = \sum_{xOR=LEAR}^{REAR} (xMax-xOR+1)(yMax-yOR+1)
                //    (xMax-xOR+1) is the number of horizontal translations
                //    (yMax-yOR+1) is the number of vertical translations
                // Closed form: (1/2)(2(xMax+1)(REAR-LEAR+1)-REAR(REAR+1)+(LEAR-1)LEAR)(yMax-yOR+1)
                counter[ACUTE] += (2*(xMax + 1)*(REAR - LEAR + 1) - REAR*(REAR + 1) + (LEAR - 1)*LEAR)*(yMax - yOR + 1)/2;
            } // if

            // Right obtuse range
            if (LEROR <= xMax) {
                // Answer = \sum_{xOR=LEROR}^{xMax} (xMax-xOR+1)(yMax-yOR+1)
                //    (xMax-xOR+1) is the number of horizontal translations
                //    (yMax-yOR+1) is the number of vertical translations
                // Closed form: (1/2)(2(xMax+1)(xMax-LEROR+1)-xMax(xMax+1)+(LEROR-1)LEROR)(yMax-yOR+1)
                counter[OBTUSE] += (2*(xMax + 1)*(xMax - LEROR + 1) - xMax*(xMax + 1) + (LEROR - 1)*LEROR)*(yMax - yOR + 1)/2;
            } // if
        } // for yOR
    } // subsubcase1C3_Region3(long,long,long[],int,int,int)
	//---------------------------------------------------------------
    // CASE 2: two vertices are on the left edge
    // Complexity: O(n^2)  (because of Subcase 2-C; would be O(1) otherwise)
    static void case2(long xMax, long yMax, long[] counter) {
        // Subcase 2-A: yBottom = 0, yOther = 0 (right triangle)
        // Answer = \sum_{yTop=1}^yMax \sum_{xOther=1}^{xMax} (xMax-xOther+1)(yMax-yTop+1)
        //    (xMax-xOther+1) is the number of horizontal translations
        //    (yMax-yTop+1) is the number of vertical translations
        // Closed form: (1/4)(xMax)(xMax+1)(yMax)(yMax+1)
        // Complexity: O(1)
        long subcase2A = xMax*(xMax + 1)*yMax*(yMax + 1)/4;
        counter[RIGHT] += subcase2A;

        // Subcase 2-A-flip: yBottom = 0, yOther = yTop (right triangle)
        // - mirror image of Subcase 2-A (flip top to bottom)
        // Complexity: O(1)
        counter[RIGHT] += subcase2A;

        // Subcase 2-B: yBottom = 0, yOther > yTop (obtuse triangle)
        // Answer = \sum_{yTop=1}^{yMax-1} \sum_{xOther=1}^{xMax} \sum_{yOther=yTop+1}^{yMax} (xMax-xOther+1)(yMax-yOther+1)
        //    (xMax-xOther+1) is the number of horizontal translations
        //    (yMax-yOther+1) is the number of vertical translations
        // Closed form: (1/12)(xMax)(xMax+1)(yMax-1)(yMax)(yMax+1)
        // Complexity: O(1)
        long subcase2B = xMax*(xMax + 1)*(yMax - 1)*yMax*(yMax + 1)/12;
        counter[OBTUSE] += subcase2B;

        // Subcase 2-B-flip: yBottom > 0, yOther = 0 (obtuse triangle)
        // - mirror image of Subcase 2-B (flip top to bottom)
        // Complexity: O(1)
        counter[OBTUSE] += subcase2B;

        // Subcase 2-C: yBottom = 0 < yOther < yTop
        // Complexity: O(n^2)
        subcase2C(xMax, yMax, counter);

    } // case2(long,long,long[])
	//---------------------------------------------------------------
    // Subcase 2-C: yBottom = 0 < yOther < yTop
    // - in every row with y = yOther, there is at most one point (with integer coordinates)
    //   that gives a right triangle (point on the circle with diameter equal to the segment
    //   joining (0,0) and (0,yTop)); every point to the left of this gives an obtuse triangle,
    //   every point to the right gives an acute triangle
    // - note that if x is a non-negative int, then (int)Math.sqrt(x*x) always = x (unless the squaring causes overflow),
    //   i.e., we don't need to worry that Math.sqrt(x*x) is slightly below x, and therefore (int)Math.sqrt(x**) = x-1
    // Complexity: O(n^2)
    static void subcase2C(long xMax, long yMax, long[] counter) {
        for (int yTop = 2; yTop <= yMax; yTop++) {   // start at 2
            for (int yOther = 1; yOther < yTop; yOther++) {   // start at 1, stop before yTop
                int xOnCircleSquared = yOther*yTop - yOther*yOther;
                int xThreshold = (int)Math.sqrt(xOnCircleSquared);
                int REOR;   // right end of obtuse range
                int LEAR;   // left end of acute range

                if (xThreshold <= xMax) {
                    // Check if x-coord on circle is an integer
                    if (xThreshold*xThreshold == xOnCircleSquared) {
                        // Right triangle
                        // (xMax-xThreshold+1) is the number of horizontal translations
                        // (yMax-yTop+1) is the number of vertical translations
                        counter[RIGHT] += (yMax-yTop+1)*(xMax-xThreshold+1);
                        REOR = xThreshold - 1;
                        LEAR = xThreshold + 1;
                    } // if
                    else {
                        // No right triangle
                        REOR = xThreshold;
                        LEAR = xThreshold + 1;
                    } // else
                } // if
                else {    // xThreshold > xMax
                    REOR = (int)xMax;
                    LEAR = (int)(xMax + 1);
                } // else

                // Obtuse triangle(s)
                // Answer = \sum_{xOther=1}^{REOR} (xMax-xOther+1)(yMax-yTop+1)
                //    (xMax-xOther+1) is the number of horizontal translations
                //    (yMax-yTop+1) is the number of vertical translations
                // Closed form: (1/2)(REOR)(2*xMax-REOR+1)(yMax-yTop+1)
                counter[OBTUSE] += REOR*(2*xMax - REOR + 1)*(yMax - yTop + 1)/2;

                // Acute triangle(s)
                // Answer = \sum_{xOther=LEAR}^{xMax} (xMax-xOther+1)(yMax-yTop+1)
                //    (xMax-xOther+1) is the number of horizontal translations
                //    (yMax-yTop+1) is the number of vertical translations
                // Closed form: (1/2)(xMax*xMax+3*xMax-2*xMax*LEAR-3*LEAR+LEAR*LEAR+2)(yMax-yTop+1)
                counter[ACUTE] += (xMax*xMax + 3*xMax - 2*xMax*LEAR - 3*LEAR + LEAR*LEAR + 2)*(yMax - yTop + 1)/2;
            } // for yOther
        } // for yTop

    } // subcase2C(long,long,long[])
	//---------------------------------------------------------------
    // CASE 3: three vertices are on the left edge, with yBottom = 0
    // Complexity: O(1)
    static void case3(long xMax, long yMax, long[] counter) {
        // Answer = \sum_{yTop=2}^yMax \sum_{yMid=1}^{yTop-1} (xMax+1)(yMax-yTop+1)
        //    (xMax+1) is the number of horizontal translations
        //    (yMax-yTop+1) is the number of vertical translations
        // Closed form: (1/6)(xMax+1)(yMax-1)(yMax)(yMax+1)
        counter[DEGENERATE] += (xMax + 1)*(yMax - 1)*yMax*(yMax + 1)/6;
    } // case3(long,long,long[])
	//---------------------------------------------------------------
    // Returns a length-2 array containing the slope of the line through
    // (x1,y2) and (x2,y2) in the form [rise, run].  The slope is a fraction,
    // rise/run, in reduced form, with the convention that the denominator
    // is never negative.  A vertical line is assigned slope 1/0 (never -1/0).
    //
    // Assumption: (x1,y1) != (x2,y2) (in which case slope is undefined)
    static int[] getSlope(int x1, int y1, int x2, int y2) {
        int rise = y2 - y1;
        int run = x2 - x1;
        if (run == 0) {   // vertical slope
            return new int[]{1, 0};
        } // if
        else {
            if (run < 0) {   // convention:  run should always be positive
                rise = -rise;
                run = -run;
            } // if
            int g = (int)gcd(Math.abs(rise), run);
            rise /= g;
            run /= g;
            return new int[]{rise, run};
        } // else
    } // getSlope(int,int,int,int)
    //--------------------------------------------------------------------
    static long gcd(long a, long b) {
        while (b != 0) {
            long rem = a % b;
            a = b;
            b = rem;
        } // while
        return a;
    } // gcd(long,long)
	//---------------------------------------------------------------
} // class AROD_Keliher

