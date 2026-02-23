import java.util.ArrayList;
import java.util.Scanner;

public class APivotalQuestion_JPB {

	public static void main(String[] args) {

		Scanner in = new Scanner(System.in);
		int n;
		
		n = in.nextInt();
		int [] a = new int[n+2];
		int [] left = new int[n+2];
		int largest = -1;
		a[0] = left[0] = largest;
		for(int i=1; i<=n; i++) {
			a[i] = in.nextInt();
			largest = left[i] = Math.max(largest, a[i-1]);
		}
		int [] right = new int[n+2];
		int smallest = Integer.MAX_VALUE;
		a[n+1] = right[n+1] = smallest;
		for(int i=n; i>=1; i--) {
			smallest = right[i] = Math.min(smallest, a[i+1]);
		}
		ArrayList<Integer> list = new ArrayList<>();
		for(int i=1; i<=n; i++) {
			if (left[i] < a[i] && right[i] > a[i]) {
				list.add(a[i]);
			}
		}
		System.out.print(list.size());
		int count = 0;
		for(int val : list) {
			System.out.print(" " + val);
			if (++count == 100)
				break;
		}
		System.out.println();

	}

}
