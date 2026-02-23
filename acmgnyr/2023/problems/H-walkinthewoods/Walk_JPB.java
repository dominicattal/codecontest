import java.util.ArrayList;
import java.util.Scanner;

public class Walk_JPB {

	public static final boolean DEBUG = false; 
	public static final int MAXK = 1000000;
	public static final int N = 0;
	public static final int E = 1;
	public static final int S = 2;
	public static final int W = 3;
	public static final char[] DIRS = {'N', 'E', 'S', 'W'};
	
	public static class Vertex
	{
		int x, y;
		Edge [] adj = new Edge[4];		// N, W, S, E
		int numEdges;
		
		public Vertex(int x, int y) {
			this.x = x;
			this.y = y;
			for(int i=0; i<adj.length; i++)
				adj[i] = null;
			numEdges = 0;
		}
		
		public String toString()
		{
			return "(" + x + "," + y + ")";
		}
	}
	
	public static class Edge
	{
		Vertex w;
		int dir;
		int level;
		int cycleNum;
		Edge match;
		
		public Edge(Vertex w, int dir, int level) {
			this(w, dir, level, null);
		}
		public Edge(Vertex w, int dir, int level, Edge match) {
			this.w = w;
			this.dir = dir;
			this.level = level;
			cycleNum = 0;
			this.match = match;
		}
		
		public String toString()
		{
			return DIRS[dir] + "-->" + w + level;
		}
	}
	
	public static Vertex[] vertices;
	
	public static void main(String[] args) {

		Scanner in = new Scanner(System.in);
		int n = in.nextInt();
		int m = in.nextInt();
		vertices = new Vertex[n+1];
		for(int i=1; i<=n; i++) {
			int x = in.nextInt();
			int y = in.nextInt();
			vertices[i] = new Vertex(x,y);
		}
		for(int i=0; i<m; i++) {
			int i1 = in.nextInt();
			int i2 = in.nextInt();
			int level = in.nextInt();
			addEdge(i1, i2, level);
		}
		int start = in.nextInt();
		char d = in.next().charAt(0);
/**/
 		printGraph();
/**/
		
		int dir=0;
		while(DIRS[dir] != d)
			dir++;
		
		Vertex v = vertices[start];
		Edge nextedge = v.adj[dir];	
		int cycleNum = 1;
		ArrayList<Edge> list = new ArrayList<>();
if (DEBUG)		System.out.println(nextedge);
		do {
			Edge edge = nextedge;
			v = edge.w;
			dir = edge.dir;
			nextedge = nextEdge(v, dir);
			if (edge.cycleNum == cycleNum) {
if(DEBUG)				System.out.println("  removing cycle " + cycleNum);
				processCycle(list, edge);
				cycleNum++;
			}
			edge.level--;
			edge.match.level--;
if (DEBUG)			System.out.println("  subtracting 1 from " + edge);
			if (edge.level <= 0) {
				removeEdge(edge);
				list.clear();
				cycleNum++;
			}
			else {
				edge.cycleNum = cycleNum;
				list.add(edge);
			}
if (DEBUG)			System.out.println(nextedge + " using " + v + " and " + DIRS[dir]);
if (DEBUG) {
	String s;
	s = in.nextLine();
}
		} while(nextedge != null);
		System.out.println(v.x + " " + v.y);
	}
	
	public static void removeEdge(Edge e)
	{
		Vertex w = e.w;
		int i=0;
		while(w.adj[i] != e.match)
			i++;
		Vertex v = w.adj[i].w;
if (DEBUG)		System.out.println("removing edge " + DIRS[w.adj[i].dir] + " to " + v);
		w.adj[i] = null;
		w.numEdges--;
		i=0;
		while(v.adj[i] != e)
			i++;
if (DEBUG)		System.out.println("removing edge " + DIRS[v.adj[i].dir] + " to " + v.adj[i].w);
		v.adj[i] = null;
		v.numEdges--;
	}
	
	public static void processCycle(ArrayList<Edge> list, Edge e)
	{
										// find lowest level on this cycle
		int istart=list.indexOf(e);
		int cnum = e.cycleNum;
		int min = MAXK+1;
		int doublemin = 3*MAXK;
		for(int i=istart; i<list.size(); i++) {
			e = list.get(i);
			if (list.indexOf(e.match) > i) {		// same edge in loop but going in opposite direction
				if (e.level < doublemin)
					doublemin = e.level;
			}
			else if (e.level < min)
				min = e.level;
			if (e.cycleNum != cnum) {			// if this happens, there's an error somewhere
				System.out.println("error with cycle numbers on list");
				System.exit(-1);
			}
		}
										// remove all but the last time through cycle
if (DEBUG) System.out.println("  min, doublemin = " + min + " " + doublemin);
		min = Math.min(min-1, (doublemin+1)/2 -1);
		for(int i=istart; i<list.size(); i++) {
			e = list.get(i);
			e.level -= min;
			e.match.level -= min;
		}
		printGraph();
		list.clear();
	}
	
	public static void addEdge(int i1, int i2, int level)
	{
		Vertex v1 = vertices[i1];
		Vertex v2 = vertices[i2];
		if (v1.x == v2.x) {
			if (v1.y > v2.y) {				// Edge going S from v1 to v2
				Edge e1 = v1.adj[S] = new Edge(v2, S, level);
				Edge e2 = v2.adj[N] = new Edge(v1, N, level, e1);
				e1.match = e2;
			}
			else {							// Edge going N from v1 to v2
				Edge e1 = v1.adj[N] = new Edge(v2, N, level);
				Edge e2 = v2.adj[S] = new Edge(v1, S, level, e1);
				e1.match = e2;
			}
		}
		else if (v1.x < v2.x) {				// Edge going E from v1 to v2
			Edge e1 = v1.adj[E] = new Edge(v2, E, level);
			Edge e2 = v2.adj[W] = new Edge(v1, W, level, e1);
			e1.match = e2;
		}
		else {							// Edge going W from v1 to v2
			Edge e1 = v1.adj[W] = new Edge(v2, W, level);
			Edge e2 = v2.adj[E] = new Edge(v1, E, level, e1);
			e1.match = e2;
		}
		v1.numEdges++;
		v2.numEdges++;
	}
	
	public static Edge nextEdge(Vertex v, int dir)		// edge just entered v in direction dir - return next edge
	{
if (DEBUG) System.out.println("  nextEdge: " + v + " " + dir + ", numedges = " + v.numEdges);
		if (v.numEdges == 4)
			return v.adj[dir];
		else {
			int nextDir = (dir+3)%4;
			int count = 0;
			while(count < 3 && v.adj[nextDir] == null) {
				nextDir = (nextDir+1)%4;
				count++;
			}
			if (count == 3)
				return null;
			return v.adj[nextDir];
		}
	}

	public static void printGraph() {
		if (!DEBUG) return;
		for(int i=1; i<vertices.length; i++) {
			System.out.print(vertices[i]);
			for(Edge e : vertices[i].adj) {
				System.out.print("," + e);
			}
			System.out.println();
		}
	}
}
