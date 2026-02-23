/*
 * Forest for the trees 
 * Solution by John Buck, Greater NY 
 */
#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>

#define		MAX_COORD	100000000

using namespace std;

struct STree {
	STree() {
		x = 0;
		y = 0;
	}
	STree(int xt, int yt) {
		x = xt;
		y = yt;

	}
	bool equals(const STree &c) const {
		return(x == c.x && y == c.y);
	}
	int x;
	int y;
};

struct TreeHash {
	std::size_t operator()(const STree &t) const
	{
		/* Normalize */
		std::size_t x = t.x + MAX_COORD;
		std::size_t y = t.y + MAX_COORD;
		return(y * MAX_COORD * 2 + x);
	}
};

struct TreeEqual {
	bool operator()(const STree& l, const STree& r) const
	{
		return l.equals(r);
	}
};

struct SSensor {
	int xdist;
	int ydist;
};

#define	NDIR		4

struct SDirTrnMatrix {
	int xxmul;
	int xymul;
	int yxmul;
	int yymul;
} dirmat[NDIR] = {
	{  1,  0,  0,  1}, // Robot pointing N
	{  0,  1, -1,  0}, // Robot pointing E
	{ -1,  0,  0, -1}, // Robot pointing S
	{  0, -1,  1,  0}  // Robot pointing W
};



unordered_map<STree, const STree *, TreeHash, TreeEqual> forest;

SSensor **sensors;
int nt, ns, rmax;

/*
 * *pt = tree to check as the anchor
 * dir = direction relative to robot
 * rlog = result, if location is consistent
 */
bool CheckTree(const STree* pt, int dir, STree &rloc)
{
	int i;
	STree tf, tt;
	std::unordered_map<STree, const STree *>::const_iterator got;
	std::unordered_map<STree, const STree *, TreeHash, TreeEqual> match;

	/* Is this tree a candidate? 
	 * Get coords of potential location of robot relative to the tree
	 */
	tf.x = pt->x - (dirmat[dir].xxmul * sensors[0]->xdist + dirmat[dir].xymul * sensors[0]->ydist);
	tf.y = pt->y - (dirmat[dir].yxmul * sensors[0]->xdist + dirmat[dir].yymul * sensors[0]->ydist);
	if ((got = forest.find(tf)) != forest.end()) {
		/* Nope, tree there already */
		return(false);
	}
	match.insert(std::make_pair(*pt, pt));
	for (i = 1; i < ns; i++) {
		/* Check if sensor in range of tf */
		tt.x = tf.x + (dirmat[dir].xxmul * sensors[i]->xdist + dirmat[dir].xymul * sensors[i]->ydist);
		tt.y = tf.y + (dirmat[dir].yxmul * sensors[i]->xdist + dirmat[dir].yymul * sensors[i]->ydist);
		if ((got = forest.find(tt)) != forest.end()) {
			/* Tree where it should be */
			match.insert(*got);
		}
		else {
			/* No tree there, this is not a candidate */
			return(false);
		}
	}
	/*
	 * We have populated match with all the trees that the robot sees
	 * See if any other trees are within range that the robot missed.  If so,
	 * then this is not a candidate.
	 */
	for (auto it = forest.begin(); it != forest.end(); it++) {
		if (match.find(it->first) == match.end()) {
			/* Tree should not be seen by robot - check distance */
			if (abs(tf.x - it->first.x) + abs(tf.y - it->first.y) <= rmax) {
				/* not a candidate */
				return(false);
			}
		}
	}
	rloc = tf;
	return(true);
}

int main(int argc, char** argv)
{
	int i, x, y;
	STree *t, robot_location;
	bool bfound = false;

	if(scanf("%d %d %d", &(nt), &(ns), &(rmax)) != 3){
		fprintf(stderr, "Whoops! not enough input on line 1\n");
		return(1);
	}

	for (i = 0; i < nt; i++) {
		if(scanf("%d %d", &(x), &(y)) != 2){
			fprintf(stderr, "Whoops! not enough input for tree %d\n", i);
			return(2);
		}
		t = new STree(x, y);
		forest.insert(std::make_pair(*t, t));
	}
	sensors = new SSensor *[ns];
	for (i = 0; i < ns; i++) {
		sensors[i] = new SSensor();
		if(scanf("%d %d", &(sensors[i]->xdist), &(sensors[i]->ydist)) != 2){
			fprintf(stderr, "Whoops! not enough input for sensor %d\n", i);
			return(2);
		}
	}

	for (std::unordered_map<STree, const STree*>::const_iterator t = forest.begin(); t != forest.end(); t++) {
		for (i = 0; i < NDIR; i++) {
			if (CheckTree(t->second, i, robot_location) == true) {
				if (bfound) {
					printf("Ambiguous\n");
					return(0);
				}
				bfound = true;
			}
		}
	}
	if (bfound == false) {
		printf("Impossible\n");
	}
	else {
		printf("%d %d\n", robot_location.x, robot_location.y);
	}
	return(0);
}

