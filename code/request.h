#ifndef _REQUEST_H_
#define _REQUEST_H_

#include "includes.h"
const double inf = 1e6;
const double eps = 1e-6;

struct taxi_t;
struct node_t;

struct request_t
{
	int id;
	int time;
	int pickup, dropoff;
	int count;
	double detour, pickup_dist, service_time;
	double dist_sd;
};
extern request_t request;

struct pair_t {
	int groupid;
	int taxiid;
	vector<node_t*> schedule;
	vector<int> path;
	bool update;
	bool stop;
	double dist;
	double serve_dist;
	double serve_shortest_dist;

	pair_t(int groupid, int taxiid, vector<node_t*> schedule, vector<int> path, bool update,
		bool stop, double dist, double serve_dist, double serve_shortest_dist)
		:groupid(groupid), taxiid(taxiid), schedule(schedule), path(path), update(update), stop(stop), serve_dist(serve_dist), serve_shortest_dist(serve_shortest_dist)
	{
	}
};
extern map<int, vector<pair_t>> schedule_taxi_pair;

struct result_t
{
	double regret, serve_regret, revenue_regret;
	double deregret;
	double serve_dist, unserve_dist, pickup_dist, service_time;
	int serve_count, unserve_count;
	double add_serve_dist;
	double detour;
	int taxiid;
	vector<node_t*> schedule;
	int pick_pos, drop_pos;
	result_t(double regret, double serve_regret, double revenue_regret,
		double deregret,
		double serve_dist, double unserve_dist,
		int serve_count, int unserve_count,
		double add_serve_dist, double detour,
		int taxiid, vector<node_t*> schedule,
		int pick_pos, int drop_pos) :
		regret(regret),
		serve_regret(serve_regret),
		revenue_regret(revenue_regret),
		deregret(deregret),
		serve_dist(serve_dist),
		unserve_dist(unserve_dist),
		serve_count(serve_count),
		unserve_count(unserve_count),
		add_serve_dist(add_serve_dist),
		detour(detour),
		taxiid(taxiid),
		schedule(schedule),
		pick_pos(pick_pos),
		drop_pos(drop_pos)
	{
	}
	bool operator<(const result_t& y) const
	{
		if (fabs(deregret - y.deregret) > eps) {
			return deregret > y.deregret;
		}
		else if (fabs(detour - y.detour) > eps) {
			return detour < y.detour;
		}
		else {
			return taxiid < y.taxiid;
		}
	}
};
extern set<result_t> result;
extern set<result_t> singleresult;
typedef set<result_t>::iterator result_ptr;
#ifdef DEBUG
extern int request_id[];
#endif

extern vector<vector<int>> vis_grid[2];
extern int cur_request_count;
extern int cur_unserve_count;
extern int cur_serve_count;
extern double cur_allreq_dist;
extern double cur_unserve_dist;
extern double cur_serve_dist;
extern double cur_serve_shortest_dist;
extern double cur_detour;

bool prune(double deregret, double detour);
void insert_result(int pick_pos, int drop_pos, double add_served_dist);
void read_request(queue<request_t>& requests);
void request_insertion();
void generate_trip(int reqpos, int passenger, double serve_dist, double serve_shortest_dist);
void greedy_simulate(queue<request_t>& requests, vector<taxi_t>& taxi);
void replace_simulate(queue<request_t>& requests, vector<taxi_t>& taxis);
void optimal_simulate(queue<request_t>& requests, vector<taxi_t>& taxis);
#endif // !_REQUEST_H_
