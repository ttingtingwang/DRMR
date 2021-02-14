#ifndef _TAXI_H_
#define _TAXI_H_

#include "includes.h"
#include "vertex.h"
#include "request.h"
using namespace std;

struct node_t;
struct taxi_t;
extern int count_node;
typedef pair<int, int> grid_id;

struct parent_t
{
	int id, vert, count, passenger;
	double limit, dist_parent, dist_root;
	double dist_slack; 
	bool start, onboard;

	parent_t(int id, int vert, int count, int passenger, 
		double limit, double dist_parent, double dist_root,
		bool start, bool onboard)
		: id(id), vert(vert), count(count), passenger(passenger), 
		limit(limit), dist_parent(dist_parent), dist_root(dist_root),
		start(start), onboard(onboard)
	{
	}
};

struct node_t
{
	int id, vert, count, passenger;
	parent_t* parent;
	double limit, dist_parent, dist_root;
	double dist_slack;
	bool start, onboard;

	node_t(){}
	//for root
	node_t(int vert, int passenger, int count);
	//for request
	node_t(long id, int vert, int count, bool start, double limit);
	//for building the tree
	node_t(node_t* old_node, parent_t* parent, double dist_parent);
	~node_t();
};

struct taxi_t
{
	int id;
	vector<int> cur_path;
	vector<node_t*> root,  root_temp;
	double dist;
	bool stop, update;
	double served_dist;
	double shortest_dist;
	grid_id gid;

	taxi_t() {}
};

extern taxi_t cur_taxi;
extern vector<taxi_t> taxis;
extern pair<node_t*, node_t*> request_node;
extern double taxi_dist;

node_t* get_new_node(int position);
void calc_slack(int position);
void insert_sd_end(int position);
void insert_s(int position);
void insert_d(int pick_pos, int position);
void insert_sd(int position);
void insert_d_end(int pick_pos, int position);
int prune_s(int position);
bool prune_d(int position);
int prune_sd(int position);
vector<taxi_t> set_taxi(int n);
taxi_t init(int vertex);
vector<int> filter_taxi();
void push(vector<node_t*> schedule);
bool insert();
void move();
void arrive();
void pickup(long id, int position);
double get_next_vert();
void evaluate();
bool check_limit(int position);

#endif