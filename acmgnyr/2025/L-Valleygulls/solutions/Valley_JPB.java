import java.util.ArrayList;
import java.util.Scanner;

public class Valley_JPB {

	public static final int MAXP = 100;
	public static final double TOL = 1e-5;

	public static int compare(double a, double b)
	{
		if (Math.abs(a-b) <= TOL)
			return 0;
		else if (a < b)
			return -1;
		else
			return 1;
	}


	public static class Point
	{
		double x, y;

		public Point(double x, double y)
		{
			this.x = x;
			this.y = y;
		}

		public String toString()
		{
			return "(" + x + "," + y + ")";
		}
	}

	public static class Spline
	{
		int n;				// number of quadratics
		Line [] lines;

		public Spline(int n)
		{
			this.n = n;
			lines = new Line[n];
		}

		public void setLine(int i, Line q)
		{
			lines[i] = q;
		}

		public Point[] maxima()
		{
			ArrayList<Point> list = new ArrayList<>();
			boolean prevHigher = false;
			for(Line line : lines) {
				if (line.p1.y > line.p2.y) {
					if (!prevHigher)
						list.add(line.p1);
					prevHigher = true;
				}
				else
					prevHigher = false;
			}
			if (lines[n-1].p2.y > lines[n-1].p1.y)
				list.add(lines[n-1].p2);
			return list.toArray(new Point[0]);
		}

		public Point nextPointRight(Point p)
		{
			int i=0;
			while (i < n) {
				Point pr = lines[i].pointForY(p.y);
				if (pr != null && compare(pr.x, p.x) > 0)
					return pr;
				i++;
			}
			return new Point(lines[n-1].p2.x, p.y);
		}

		public Point nextPointLeft(Point p)
		{
			int i=n-1;
			while (i >= 0) {
				Point pr = lines[i].pointForY(p.y);
				if (pr != null && compare(pr.x, p.x) < 0)
					return pr;
				i--;
			}
			return new Point(lines[0].p1.x, p.y);
		}

		public double fillArea(Point left, Point right) // left.y = right.y
		{
			int i=0;
			while (i < n-1 && compare(lines[i].p2.x,left.x) < 0)
				i++;
			double area = 0.0;
			double start = left.x;
			while (i < n && compare(lines[i].p2.x,right.x) <= 0) {
				area += lines[i].area(start, lines[i].p2.x);
				start = lines[i].p2.x;
				i++;
			}
			if (i < n)
				area += lines[i].area(start,  right.x);
			return left.y*(right.x-left.x) - area;
		}
	}

	private static class Line
	{
		Point p1, p2;
		double b, m;		// mx + b

		public Line(Point p1, Point p2)
		{
			this.p1 = p1;
			this.p2 = p2;
			m = (p1.y-p2.y)/(p1.x-p2.x);
			b = p1.y - m*p1.x;
		}
		
		public double eval(double x)
		{
			return b + m*x;
		}

		public double slope(double x)
		{
			return m;
		}

		public double area(double low, double high)
		{
			double upper = m*high*high/2 + b*high;
			double lower = m*low*low/2 + b*low;
			return upper-lower;
		}

		public Point max()
		{
			return (p1.y > p2.y) ? p1 :p2;
		}

		public Point pointForX(double x)
		{
			if (x < p1.x || x > p2.x)
				return null;
			double y = m*x+b;
			return new Point(x,y);
		}

		public Point pointForY(double y)
		{
			double x = (y-b)/m;
			if (x < p1.x || x > p2.x)
				return null;
			return new Point(x,y);
		}
	}

	public static class Node
	{
		Point pleft, pright;		// left, right intersection points
		int maxi = -1;				// index of maxima (either pleft or pright)
		double rleft, rright;		// x range of rain filling this node (one = maxima.x)
		Node lnode, rnode;			// left child, right child
		double area;				// area to fill
		double currfill;			// amount currently filled
		ArrayList<Integer> inests;	// index to nest array of nests covered by this node.

		public Node()
		{
			this(null, null);
		}

		public Node(Node left, Node right)
		{
			lnode = left;
			rnode = right;
			inests = new ArrayList<>();
		}

		public double getMinTimeToFill()
		{
			if (lnode == null && rnode == null) {
				//System.out.println("area, currfill, rleft, right = " + area + "," + currfill + "," + rleft + "," + rright);
				return (area - currfill)/(rate*(rright-rleft));
			}
			else if (lnode == null)
				return rnode.getMinTimeToFill();
			else if (rnode == null)
				return lnode.getMinTimeToFill();
			else
				return Math.min(lnode.getMinTimeToFill(), rnode.getMinTimeToFill());
		}

		public boolean applyTime(double t)
		{
			if (lnode == null && rnode == null) {
				for(Integer inest : inests) {
					if (nestArea[inest] > currfill) {
						double fillt = (nestArea[inest]-currfill)/rate/(rright-rleft);
						if (compare(fillt, t) <= 0)
							nestTimes[inest] = globalt + fillt;
					}
				}
				currfill += t*rate*(rright-rleft);
				if (compare(currfill-area, 0.0) == 0)
					return true;
			}
			else if (lnode == null) {
				boolean rdone = rnode.applyTime(t);
				if (rdone) {
					currfill += rnode.area;
					rnode = null;
				}
			}
			else if (rnode == null) {
				boolean ldone = lnode.applyTime(t);
				if (ldone) {
					currfill += lnode.area;
					lnode = null;
				}
			}
			else {
				boolean ldone = lnode.applyTime(t);
				boolean rdone = rnode.applyTime(t);
				if (ldone && rdone) {
					currfill += lnode.area;
					currfill += rnode.area;
					lnode = null;
					rnode = null;
				}
				else if (ldone) {
					Node p = rnode;
					while (p != null) {
						p.rleft = lnode.rleft;
						if (p.lnode == null)
							p = p.rnode;
						else
							p = p.lnode;
					}
					currfill += lnode.area;
					lnode = null;
				}
				else if (rdone) {
					Node p = lnode;
					while (p != null) {
						p.rright = rnode.rright;
						if (p.rnode == null)
							p = p.lnode;
						else
							p = p.rnode;
					}
					currfill += rnode.area;
					rnode = null;
				}
			}
			return false;
		}

		public boolean insertNest(int inest)
		{
			if (lnode != null && lnode.insertNest(inest))
				return true;
			if (rnode != null && rnode.insertNest(inest))
				return true;
			double x = nests[inest].x;
			if (x < pleft.x || x > pright.x)
				return false;
			inests.add(inest);					// still need to handle it x is at peak
			//			System.out.println("inserting nest between " + pleft + " and " + pright);
			return true;
		}

		public void print()
		{
			print("");
		}
		public void print(String level)
		{
			if (lnode != null)
				lnode.print(level+"L");
			if (rnode != null)
				rnode.print(level+"R");
			System.out.print("level: " + level + ": ");
			System.out.print(pleft + "--" + pright);
			if (maxi != -1)
				System.out.print(": " + maxima[maxi]);
			System.out.println();
			System.out.println("  area = " + area + "    currfill = " + currfill + "   t = " + (area -currfill)/(rate*(rright-rleft)));
			System.out.println("  rain range = " + rleft + "--" + rright);
		}
	}

	public static int npoints;
	public static Point[] points;
	public static Line[] lines;
	public static Spline s;
	public static double h[];
	public static Point[] maxima;	// location of maxima in spline
	public static Point[] left;		// next point to left of maxima at same height
	public static Point[] right;	// ditto for right
	public static double rate;		// rain rate, feet/sec
	public static Point[] nests;	// nest locations;
	public static double [] nestArea;	// fill area which reaches each nest.
	public static double [] nestTimes;	// time water reaches nest
	public static double globalt;		// latest time when an area was filled;

	public static void main(String[] args) {

		Scanner in = new Scanner(System.in);
		npoints = in.nextInt();
		rate = in.nextDouble();
		int m = in.nextInt();
		//		rate = 1.0;
		int n = npoints-1;						// number of lines
		s = new Spline(n);
		points = new Point[npoints];
		lines = new Line[npoints];		// really only need n
		h = new double[npoints];
		for(int i=0; i<npoints; i++) {
			double x = in.nextDouble();
			double y = in.nextDouble();
			points[i] = new Point(x,y);
			if (i > 0) {
				lines[i-1] = new Line(points[i-1], points[i]);
				s.setLine(i-1, lines[i-1]);
			}
		}
		maxima = s.maxima();
		left = new Point[maxima.length];
		right = new Point[maxima.length];
		for(int i=0; i<maxima.length; i++) {
			left[i] = s.nextPointLeft(maxima[i]);
			right[i] = s.nextPointRight(maxima[i]);
//System.out.println(left[i] + "<--" + maxima[i] + "-->" + right[i]);
//System.out.println("  area = " + s.fillArea(left[i], right[i]));
		}

		Node tree = buildTree(0, maxima.length-1);
//System.out.println("maxima tree");
//tree.print();
		tree = buildTree2(tree);
//System.out.println();
//tree.print();
										// read in nest locations		
		nests = new Point[m];
		nestArea = new double[m];
		nestTimes = new double[m];
		int iq = 0;
		for(int i=0; i<m; i++) {
			int x = in.nextInt();
//System.out.println("x = " + x);
			Point p = lines[iq].pointForX(x);
			while (iq < lines.length-1 && p == null) {
				iq++;
				p = lines[iq].pointForX(x);
			}
			nests[i] = p;
//System.out.println("  p = " + p);
			if (compare(lines[iq].slope(x), 0.0) > 0) {
				Point pl = s.nextPointLeft(p);
//System.out.println("  pl = " + pl);
				nestArea[i] = s.fillArea(pl, p);
			}
			else if (compare(lines[iq].slope(x),0.0) < 0) {
				Point pr = s.nextPointRight(p);
//System.out.println("  pr = " + pr);
				nestArea[i] = s.fillArea(p, pr);
			}
			else {
				System.out.println("Error: nest at valley/peak");
				System.exit(-1);
				Point pl = s.nextPointLeft(p);
				double leftArea = s.fillArea(pl, p);
				Point pr = s.nextPointRight(p);
				nestArea[i] = Math.min(leftArea, s.fillArea(p, pr));			
			}
//System.out.println("  nest area = " + nestArea[i]);
			tree.insertNest(i);
		}

		globalt = 0.0;
		while (tree.lnode != null || tree.rnode != null) {
//System.out.println("\nGlobal time = " + globalt + ", tree:");
//tree.print();
			double deltat = tree.getMinTimeToFill();
//System.out.println("next fill time = " + deltat);
			tree.applyTime(deltat);
			globalt += deltat;
			//			System.out.println("\ncurrent time = " + globalt);
			//			tree.print();
		}

		for(int i=0; i<m; i++) {
			System.out.printf("%.8f\n", nestTimes[i]);
		}
	}

	public static Node buildTree(int low, int high)  // builds tree where nodes are maximas
	{
		if (low > high)
			return null;
		else if (low == high) {
			Node leaf = new Node();
			leaf.maxi = low;
			return leaf;
		}
		Node node = new Node();
		int maxi=low;
		for(int i=low+1; i<=high; i++) {
			if (maxima[i].y > maxima[maxi].y)
				maxi = i;
		}
		node.lnode = buildTree(low, maxi-1);
		node.rnode = buildTree(maxi+1, high);
		node.maxi = maxi;
		return node;
	}

	public static Node buildTree2(Node root)	// uses tree from buildTree to build tree of fill areas
	{
		if (root == null)
			return new Node();
		Node lnode = buildTree2(root.lnode);
		//		System.out.println("root.maxi = " + root.maxi);
		lnode.pleft = left[root.maxi];
		lnode.pright = maxima[root.maxi];
		if (lnode.pleft.x == lnode.pright.x)
			lnode = null;
		else {
			lnode.area = s.fillArea(lnode.pleft, lnode.pright);
			lnode.currfill = 0.0;
			lnode.rright = lnode.pright.x;
			int i=root.maxi-1;
			while(i > 0 && (maxima[i].x > lnode.rright || maxima[i].y < lnode.pright.y))
				i--;
			if (i == -1)
				lnode.rleft = left[0].x;
			else
				lnode.rleft = Math.min(lnode.pleft.x, maxima[i].x);
			lnode.maxi = root.maxi;
		}
		Node rnode = buildTree2(root.rnode);
		rnode.pleft = maxima[root.maxi];
		rnode.pright = right[root.maxi];
		if (rnode.pleft.x == rnode.pright.x)
			rnode = null;
		else {
			rnode.area = s.fillArea(rnode.pleft, rnode.pright);
			rnode.currfill = 0.0;
			rnode.rleft = rnode.pleft.x;
			int i=root.maxi+1;
			while(i < maxima.length-1 && (maxima[i].x < rnode.rleft || maxima[i].y < rnode.pleft.y))
				i++;
			if (i == maxima.length)
				rnode.rright = right[maxima.length-1].x;
			else
				rnode.rright = Math.max(rnode.pright.x, maxima[i].x);
			rnode.maxi = root.maxi;
		}
		Node newRoot = new Node(lnode, rnode);
		return newRoot;
	}
}


