import java.util.*;
import java.io.*;

/**
 * Count acute, right, and obtuse triangles that fit exactly within each w*h box.
 *
 * The cases are: 
 * - 3 vertices in corners.
 * - 2 vertices on adjacent corners. The third vertex must be on the opposite edge.
 * - 2 vertices on opposite corners. The third vertex can be anywhere in the box that is not a corner and not on the diagonal.
 * - 1 vertex in a corner, 2 vertices on the non-incident edges.
 *      For this case, try each position along one edge for the second vertex. The third vertex will form either a right, acute,
 *      or obtuse triangle depending on where it lies on that edge. Consider where the circle with diameter given by the line
 *      between the bounding box corner and the first vertex, intersects the other edge (if at all).
 * Time complexity: O(XY(X+Y))
 *
 * @author Finn Lidbetter
 */

public class arod_finn {

    static double EPS = 1e-9;

    static long gcd(long a, long b) {
        return (b==0) ? a : gcd(b,a%b);
    }

    public static void main(String[] args) throws IOException {
        BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
        String[] s = br.readLine().split(" ");
        int X = Integer.parseInt(s[0]);
        int Y = Integer.parseInt(s[1]);
        long aCount = 0;
        long rCount = 0;
        long oCount = 0;
        long dCount = 0;
        for (int w=1; w<=X; w++) {
            for (int h=1; h<=Y; h++) {
                //System.out.printf("w=%d, h=%d\n",w,h);
                long aBox = 0;
                long rBox = 0;
                long oBox = 0;
                // Triangles with vertices in 3 corners of the h * w bounding box.
                rBox += 4;
                
                // Triangles with vertices in 2 opposite corners of the h * w bounding box.
                // Include points on the interior of the bounding box that are not on the diagonal.
                oBox += 2*((w+1)*(h+1) - 4 - gcd(w,h) + 1);
                
                // Triangles with vertices in 2 adjacent corners of the h * w bounding box. The other vertex must be on the opposite edge.
                for (int v=1; v<w; v++) {
                    long dx1 = -v;
                    long dy1 = h;
                    long dx2 = w-v;
                    long dy2 = h;
                    long product = dot(dx1, dy1, dx2, dy2);
                    if (product == 0) {
                        rBox += 2;
                    } else if (product < 0) {
                        oBox += 2;
                    } else {
                        aBox += 2;
                    }
                }
                for (int v=1; v<h; v++) {
                    long dx1 = -v;
                    long dy1 = w;
                    long dx2 = h-v;
                    long dy2 = w;
                    long product = dot(dx1, dy1, dx2, dy2);
                    if (product == 0) {
                        rBox += 2;
                    } else if (product < 0) {
                        oBox += 2;
                    } else {
                        aBox += 2;
                    }
                }
                long aBoxPrev = aBox;
                long rBoxPrev = rBox;
                long oBoxPrev = oBox;
                
                if (h >= 2) {
                    // Triangles with one vertex in a corner of the h * w bounding box. The other two vertices must be on the two other edges.
                    for (int v=1; v<w; v++) {
                        // Place the second vertex at each integer coordinate along the width of the box.

                        // Find the position at which the vertex on the height of the box switches from obtuse to acute.
                        // y=mx+b
                        // m = -v/h, 0 = (-v/h)*(w-v) + b, b=(v/h)*(w-v)
                        double p1 = (double)(w*v - v*v)/h;
                        if (p1 >= h - EPS) {
                            oBox += 4 * (h - 1);
                            continue;
                        }
                        int p1Floor = (int)Math.floor(p1 + EPS);
                        // p1Floor <= h.
                        
                        if (p1Floor == h) {
                            oBox += 4 * (h - 1);
                            continue;
                        }
                        // p1Floor < h

                        oBox += 4 * p1Floor;
                        if (Math.abs(p1Floor - p1) < EPS) {
                            rBox += 4;
                            oBox -= 4;
                        }
                        double disc = (v*v+h*h)/4.0 - (w-v/2.0)*(w-v/2.0);
                        if (Math.abs(disc) < EPS) {
                            aBox += 4 * (h - 1 - p1Floor);
                            if (h%2==0) {
                                rBox += 4;
                                aBox -= 4;
                            }                         
                        } else if (disc < 0) {
                            aBox += 4 * (h - 1 - p1Floor);
                        } else {
                            double p2 = (h/2.0) - Math.sqrt(disc);
                            double p3 = (h/2.0) + Math.sqrt(disc);
                            int p2Floor = (int)Math.floor(p2 + EPS);
                            int p3Floor = (int)Math.floor(p3 + EPS);
                            if (p2Floor >= h) {
                                aBox += 4 * (h - 1 - p1Floor);
                                continue;
                            }
                            aBox += 4 * (p2Floor - p1Floor);
                            if (Math.abs(p2Floor - p2) < EPS) {
                                aBox -= 4;
                                rBox += 4;
                            }
                            if (p3Floor >= h) {
                                oBox += 4 * (h - 1 - p2Floor);
                                continue;
                            }
                            oBox += 4 * (p3Floor - p2Floor);
                            if (Math.abs(p3Floor - p3) < EPS) {
                                oBox -= 4;
                                rBox += 4;
                            }
                            aBox += 4 * (h - 1 - p3Floor);
                        }
                    }
                }
                long aDelta = aBox - aBoxPrev;
                long rDelta = rBox - rBoxPrev;
                long oDelta = oBox - oBoxPrev;
                
                //System.out.printf("aBox: %d, rBox: %d, oBox: %d, aDelta: %d, rDelta: %d, oDelta: %d\n",aBox, rBox, oBox, aDelta,rDelta,oDelta);
                
                long boxCopies = (X - w + 1) * (Y - h + 1);
                //System.out.printf("Box copies: %d\n",boxCopies);
                aCount += aBox * boxCopies;
                rCount += rBox * boxCopies;
                oCount += oBox * boxCopies;
            }
        }
        long lX = (long)X;
        long lY = (long)Y;
        long nTotalWays = (((lX+1)*(lY+1))*((lX+1)*(lY+1)-1)*((lX+1)*(lY+1)-2))/6;
        dCount = nTotalWays - (aCount + rCount + oCount);
        System.out.println(aCount);
        System.out.println(rCount);
        System.out.println(oCount);
        System.out.println(dCount);
    }

    static long dot(long dx1, long dy1, long dx2, long dy2) {
        return dx1 * dx2 + dy1 * dy2;
    }
}
