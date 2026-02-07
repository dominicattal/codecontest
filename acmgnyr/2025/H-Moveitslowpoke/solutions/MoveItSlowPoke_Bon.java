import java.util.ArrayList;
import java.util.Comparator;
import java.util.PriorityQueue;
import java.util.Scanner;

public class MoveItSlowPoke_Bon {

	public static class Edge
	{
		int w;		// other vertex
		int cost;	// edge cost
		boolean used;
		
		public Edge(int w, int cost)
		{
			this.w = w;
			this.cost = cost;
			this.used = false;
		}
	}
	
	public static class Vertex
	{
		int id;
		ArrayList<Edge> adj;
		
		public Vertex(int id)
		{
			this.id = id;
			adj = new ArrayList<>();
		}
	}
	
	public static class Graph
	{
		Vertex [] vertices;
		
		public Graph(int n)
		{
			vertices = new Vertex[n+1];
			for(int i=1; i<=n; i++) {
				vertices[i] = new Vertex(i);
			}
		}
		
		public void addEdge(int v, int w, int cost)
		{
			vertices[v].adj.add(new Edge(w, cost));
			vertices[w].adj.add(new Edge(v, cost));
		}
	}
	
	public static class PQNode
	{
		int node, prev;
		Edge e;
		int dist, contDist;
		
		public PQNode(int node, Edge e, int prev, int dist, int contDist)
		{
			this.node = node;
			this.e = e;
			this.prev = prev;
			this.dist = dist;
			this.contDist = contDist;
		}
		
		public String toString()
		{
			return node + " " + prev + ":" + dist + "," + contDist;
		}
	}
	
	public static class NodeComparator implements Comparator<PQNode>
	{
		public int compare(PQNode n1, PQNode n2)
		{
			return n1.dist - n2.dist;
		}
	}
	
	public static ArrayList<Integer> [][] contTable;
	
	public static void main(String[] args) {
		Scanner in = new Scanner(System.in);
		int n = in.nextInt();
		int m = in.nextInt();
		int k = in.nextInt();
		int d = in.nextInt();
		int s = in.nextInt();
		int t = in.nextInt();
		
		contTable = new ArrayList[n+1][n+1];
		Graph g = new Graph(n);
		for(int i=0; i<m; i++) {
			int a = in.nextInt();
			int b = in.nextInt();
			int cost = in.nextInt();
			g.addEdge(a,  b,  cost);
		}
		for(int i=0; i<k; i++) {
			int a = in.nextInt();
			int b = in.nextInt();
			int c = in.nextInt();
			if (contTable[a][b] == null)
				contTable[a][b] = new ArrayList<>();
			contTable[a][b].add(c);
		}
		
		PriorityQueue<PQNode> pq = new PriorityQueue<>(new NodeComparator());
		for(Edge e: g.vertices[s].adj) {
			pq.add(new PQNode(e.w, e, s, e.cost, e.cost));
		}
		boolean found = false;
		boolean [][][] used = new boolean[n+1][n+1][n+1];
		while (!pq.isEmpty()) {
			PQNode pnode = pq.poll();
//			System.out.println(pnode);
//			String line = in.nextLine();
			if (pnode.node == t) {
				System.out.println(pnode.dist);
				found = true;
				break;
			}
			pnode.e.used = true;
			ArrayList<Integer> contV = contTable[pnode.prev][pnode.node];
			for(Edge e: g.vertices[pnode.node].adj) {
//				if (e.used)
//					continue;
				if (e.w == pnode.prev)
					continue;						// don't allow u-turns
				if (used[pnode.prev][pnode.node][e.w])
					continue;
//				System.out.println("  Processing node " + e.w);
				int newContDist = e.cost;
				if (contV != null && contV.contains(e.w)) {
					if (pnode.contDist + e.cost > d)
						continue;					// can't go down this road
					else
						newContDist = pnode.contDist + e.cost;
				}
//				else
//					newContDist = 0;
				PQNode newNode = new PQNode(e.w, e, pnode.node, pnode.dist+e.cost, newContDist);
				used[pnode.prev][pnode.node][e.w] = true;
//				System.out.println("  adding node " + newNode);
				pq.add(newNode);
			}
		}
		if (!found)
			System.out.println("impossible");
	}
	

}
