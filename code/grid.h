#ifndef _GRID_H_
#define _GRID_H_

#include "includes.h"

#define HASHINGLONG -74500000 //beijing:116000000 chengdu:103500000 manhattan: -74500000
#define HASHINGLAT 40500000  //beijing:39500000 chengdu:30500000 manhattan:40500000
//#define HASHINGLONG 103500000 //beijing:116000000 chengdu:103500000 manhattan: -74500000
//#define HASHINGLAT 30500000  //beijing:39500000 chengdu:30500000 manhattan:40500000
//#define HASHER 50
//#define RANGE 
extern int hasher, range; 


typedef pair<int, int> grid_id;
typedef pair<node_t*, taxi_t*> edge_t;


struct grid_t
{
	set<edge_t> empty_edges, tree_edges;
	//3: size of these sets
	double max_dist, min_lx, max_slack;
	int max_passenger;

	void update_tree_edge(edge_t edge, opcode op);
	void update_empty_edge(edge_t edge, opcode op);
	void update();
};
extern vector<vector<grid_t>> grid;
extern vector<vector<vector<vector<double>>>> grid_dist;
extern set<grid_t*> need_update;

struct vertex_t
{
	grid_id grid;
	double min_dis;
};
extern vertex_t Node[];

grid_id get_hash(int i);
void prepare();
void read_data();
double ldist(int s, int d);
double ldist(int s, grid_id g);
double get_dist_lx(const edge_t x);
void update_grid();

#endif // !_GRID_H_
