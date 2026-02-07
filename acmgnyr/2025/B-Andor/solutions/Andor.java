import java.util.ArrayList;
import java.util.Scanner;

public class Andor {

	public static final int MAXNODES = 100000;
	public static Scanner in = new Scanner(System.in);

	public static class Node
	{
		char type;
		boolean eval;
		int numChildren;
		int numToFlip;
		ArrayList<Node> children;

		public Node(char type, int numChildren)
		{
			this.type = type;
			this.numChildren = numChildren;
			eval = false;
			numToFlip = 0;
			children = new ArrayList<>();
		}

		public boolean addChild(Node child)
		{
			children.add(child);
			return (children.size() == numChildren);
		}
		
		public void print()				// used for debugging
		{
			System.out.println(type);
			for(Node n : children)
				n.print();
		}
	}
	
	public static void main(String[] args) {

		int n = in.nextInt();
		char type = in.next().charAt(0);
		Node tree = buildTree(n, type);
		processTree(tree);
		System.out.println(tree.numToFlip);
	}

	public static Node buildTree(int levels, char type)
	{
		int nc = in.nextInt();
		Node root = new Node(type, nc);
		ArrayList<Node> queue = new ArrayList<>();
		Node node = root;
		for(int i=2; i<=levels; i++) {
			type = (char)('A' + 'O' - type);
			int numVals = nc;
			nc = 0;
			for(int j=0; j<numVals; j++) {
				String s = in.next();
				Node newNode = null;
				try {
					int numc = Integer.parseInt(s);
					newNode = new Node(type, numc);
					queue.add(newNode);
					nc += numc;
				}
				catch (NumberFormatException e)  {
					newNode = new Node(s.charAt(0), 0);
				}
				if (node.addChild(newNode))	{	// check if this node is full
					if (!queue.isEmpty())
						node = queue.remove(0);
				}
			}
		}
		return root;
	}

	public static void processTree(Node t)
	{
		for(Node n : t.children)
			processTree(n);
		if (t.type == 'T' || t.type == 'F') {
			t.eval = t.type == 'T';
			t.numToFlip = 1;
		}
		else if (t.type == 'A') {
			boolean eval = true;
			int numToTrue = 0;
			int numToFalse = MAXNODES+1;
			for(Node node : t.children) {
				eval = eval && node.eval;
				if (node.eval)
					numToFalse = Math.min(numToFalse, node.numToFlip);
				else
					numToTrue += node.numToFlip;
			}
			t.eval = eval;
			if (t.eval)
				t.numToFlip = numToFalse;
			else
				t.numToFlip = numToTrue;
		}
		else {
			boolean eval = false;
			int numToTrue = MAXNODES+1;
			int numToFalse = 0;
			for(Node node : t.children) {
				eval = eval || node.eval;
				if (node.eval)
					numToFalse += node.numToFlip;
				else
					numToTrue = Math.min(numToTrue, node.numToFlip);
			}
			t.eval = eval;
			if (t.eval)
				t.numToFlip = numToFalse;
			else
				t.numToFlip = numToTrue;
		}
	}
}
