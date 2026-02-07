#include <iostream>
#include <vector>
#include <queue>
#include <string>

using namespace std;

struct node{
  bool is_and;
  bool is_leaf;
  bool evaluation;
  char leaf_val;
  vector<node*> children;
};


void Print_Tree(node* root){
  queue<node*> frontier;
  frontier.push(root);
  while(!frontier.empty()){
    node* cur = frontier.front();
    frontier.pop();
    if(cur->is_leaf)
      cout << "Leaf: " << cur->leaf_val << endl;
    else{
      cout << (cur->is_and? "AND" : "OR") << " Node with " << cur->children.size() << " children" << endl;
      for(int i = 0; i < cur->children.size(); i++)
	frontier.push(cur->children[i]);
    }
  }
}

			   
void Evaluate_Tree(node* root){
  if(root != NULL) // shouldn't happen
    if(root->is_leaf)
      if(root->leaf_val == 'T')
	root->evaluation = true;
      else
	root->evaluation = false;
    else{
      if(root->is_and){
	root->evaluation = true;
	for(int i = 0; i < root->children.size(); i++){
	  Evaluate_Tree(root->children[i]);
	  if(!root->children[i]->evaluation)
	    root->evaluation = false;
	}
      }
      else{ // or node
	root->evaluation = false;
	for(int i = 0; i < root->children.size(); i++){
	  Evaluate_Tree(root->children[i]);
	  if(root->children[i]->evaluation)
	    root->evaluation = true;
	}
      }
    }
}

int Num_To_Change(node* root){
  if(root == NULL) // shouldn't happen
    return 0;
  if(root->is_leaf)
    return 1;
  if((root->is_and && root->evaluation) || (!root->is_and && !root->evaluation)){
    // min of any flip
    int num_flip = 100000;
    for(int i = 0; i < root->children.size(); i++){
      // all of the evaluate nodes of the children should be true
      int cur = Num_To_Change(root->children[i]);
      if(cur < num_flip)
	num_flip = cur;
    }
    return num_flip;
  }
  else{
    // sum of all flips
    int total = 0;
    for(int i = 0; i < root->children.size(); i++)
      if(root->children[i]->evaluation == root->evaluation)
	total = total + Num_To_Change(root->children[i]);
    return total;
  }
}

  
int main(){
  queue<node*> frontier;

  node* root = new node;

  int n;
  cin >> n;
  char val;
  cin >> val;
  if(val == 'A')
    root->is_and = true;
  else
    root->is_and = false;
  int num_children;
  cin >> num_children;
  root->is_leaf = false;
  root->children.resize(num_children);
  frontier.push(root);
  while(!frontier.empty()){
    node* cur = frontier.front();
    frontier.pop();
    for(int i = 0; i < cur->children.size(); i++){
      string input;
      cin >> input;
      node* child = new node;
      if(input == "T" || input == "F"){
	child->is_leaf = true;
	child->leaf_val = input[0];
      }
      else{
	int x = stoi(input);
	child->is_leaf = false;
	child->children.resize(x);

	child->is_and = !cur->is_and;
	frontier.push(child);
      }
      cur->children[i] = child;
    }
  }

  // Print_Tree(root);
  Evaluate_Tree(root);
  cout << Num_To_Change(root) << endl;
  return 0;
}
    
