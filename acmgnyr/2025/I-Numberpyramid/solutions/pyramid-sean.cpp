#include <iostream>
#include <vector>
#include <fstream>
using namespace std;

void Print_Pyramid(const vector<vector<int> >& v){
  for(int i = 0; i < v.size(); i++){
    for(int j = 0; j < v[i].size(); j++)
      cout << v[i][j] << " ";
    cout << endl;
  }
}

	  
    
// returns the number of new cells filled, or -1 if there is a contradiction
int Pyramid_Pass(vector<vector<int> >& pyramid){
  int num_changed = 0;
  for(int i = 0; i < pyramid.size()-1; i++){
    for(int j = 0; j < pyramid[i].size(); j++){
      // Possibilty 1: both of my children exist
    
      if(pyramid[i+1][j] < 100 && pyramid[i+1][j+1] < 100){
	int new_val = pyramid[i+1][j] + pyramid[i+1][j+1];
	if(abs(new_val) >= 100){
	  //	  cout << "ERROR: Computed a " << new_val << endl;
	  return -1;
	}
	
	if(pyramid[i][j] == 100){
	  pyramid[i][j] = new_val;
	  num_changed++;
	}
	else{
	  if(pyramid[i][j] != new_val) // contradiction
	    return -1;
	}
      }
      // possibility 2: I exist and my left child exists
      if(pyramid[i][j] < 100 && pyramid[i+1][j] < 100){
	
	int new_val = pyramid[i][j]-pyramid[i+1][j];
	if(abs(new_val) > 100){
	  //	  cout << "ERROR: Computed a " << new_val << endl;
	  return -1;
	}
	if(pyramid[i+1][j+1] == 100){
	  pyramid[i+1][j+1] = new_val;
	  num_changed++;
	}
	else{
	  if(pyramid[i+1][j+1] != new_val) // contradiction
	    return -1;
	}
      }
      // possibility 3: I exist and my right child exists
      if(pyramid[i][j] < 100 && pyramid[i+1][j+1] < 100){
	int new_val = pyramid[i][j] - pyramid[i+1][j+1];
	if(abs(new_val) > 100){
	  //cout << "ERROR: Computed a " << new_val << endl;
	  return -1;
	}
	if(pyramid[i+1][j] == 100){
	  pyramid[i+1][j] = new_val;
	  num_changed++;
	}
	else{
	  if(pyramid[i+1][j] != new_val) // conradiction
	    return -1;
	}
      }
    }
  }
  return num_changed;
}
	      
	  
  

int main(){
  ifstream fin;
  fin.open("14.in");
  int n;
  cin >> n;
 
  vector<vector<int> > pyramid(n);
  for(int i = 0; i < n; i++){
    pyramid[i].resize(i+1);
    for(int j = 0; j <= i; j++)
      cin >> pyramid[i][j];

  }
  
  int result = 1;
  while(result > 0){
    result = Pyramid_Pass(pyramid);
  }

  if(result == 0){
    bool found_empty = false;
    for(int i = 0; i < pyramid.size(); i++)
      for(int j = 0; j < pyramid[i].size(); j++)
	if(pyramid[i][j] == 100)
	  found_empty = true;
    if(found_empty){
      cout << "ambiguous" << endl;
      // Print_Pyramid(pyramid);
    }
    
    else{
      cout << "solvable" << endl;
       Print_Pyramid(pyramid);
    }
  }
  if(result < 0){
    cout << "no solution" << endl;
    // Print_Pyramid(pyramid);
  }
  if(result > 0){
    cout << "solvable" << endl;
    Print_Pyramid(pyramid);
  }
  return 0;
}
 
    
      
  
  
  
