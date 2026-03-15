/*
 * Greater NY Regional 2022
 * Java version of You You See What?
 * John Buck
 * Basically, converted from C version.
 */
import java.util.Scanner;

public class uucp {
	
	public static void main(java.lang.String[] args) {
		Scanner stdin = new Scanner(System.in);
		String [] comps = stdin.nextLine().split("!");
		stdin.close();
		
		int i, j, nsame, ncomp = comps.length;
		String user = comps[ncomp-1];
		ncomp--;
		// eliminate hops to self
		for(i = 0; i < ncomp; i++){
			// count # of same
			for(j = i+1; j < ncomp; j++){
				if(comps[i].compareToIgnoreCase(comps[j]) != 0){
					break;
				}
			}
			// how many are the same?
			nsame = j - i - 1;
			if(nsame > 0){
				while(j < ncomp){
					comps[j-nsame] = comps[j];
					j++;
				}
				ncomp -= nsame;
			}
		}
		// if there are more than 2 hops then we may be able to trim
		if(ncomp > 2){
			i = 1;
			while(i < ncomp-1){
				if(comps[i-1].compareToIgnoreCase(comps[i+1]) == 0){
					// going to the same place we came from, so delete this and next
					// by copying remaining elements
					ncomp -= 2;
					for(j = i; j < ncomp; j++){
						comps[j] = comps[j+2];
					}
					if(i > 1){
						i--;
					}
				} else {
					i++;
				}
			}
		}
		for(i = 0; i < ncomp; i++){
			System.out.print(comps[i] + "!");
		}
		// always print user
		System.out.println(user);
	}
}

