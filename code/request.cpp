#include "main.h"

int cur_request_count = 0;
int cur_unserve_count = 0;
int cur_serve_count = 0;
double cur_allreq_dist = 0;
double cur_unserve_dist = 0;
double cur_serve_dist = 0;
double cur_serve_shortest_dist = 0;
double cur_detour = 0;
PerformanceTimer update_time, update_time1, match_time, all_time;
request_t request;
set<result_t> result;
set<result_t> singleresult;
double taxi_dist;
vector<int> vis_taxi[2];
vector<vector<int>> vis_grid[2];
int dir[8][2] = { { -1,-1 },{ -1,0 },{ -1,1 },{ 0,-1 },{ 0,1 },{ 1,-1 },{ 1,0 },{ 1,1 }, };
typedef pair<double, grid_id > grid_hash;
enum
{
	FROM_D = 0,
	FROM_S = 1,
};

void request_insertion()
{
	evaluate();
}

void read_request(queue<request_t>& requests)
{
	request_t req;
	//ifstream request_file((data_dir + "/request.dat").c_str());
	ifstream request_file((data_dir + "/syn_req10.dat").c_str());
	int id = 2;
	//ofstream outputfile;
	//outputfile.open(data_dir + "/output.txt");
	//outputfile << "rid " <<"time "<< "source " <<"destination "<< "dist" <<  endl;
	while (request_file >> req.time) {
		if (req.time >= logicalTimeLimit) {
			break;
		}
		if (req.time < logicalTime) {
			request_file.ignore(100, '\n');
			continue;
		}
		request_file >> req.pickup >> req.dropoff;

		request_file >> req.count;
		req.id = id;
		req.dist_sd = get_dist(req.pickup, req.dropoff);
		//outputfile << req.id / 2 <<" "<<req.time<<" "<< req.pickup<< " " << req.dropoff<< " "<< req.dist_sd<<endl;
		requests.push(req);
		id += 2;
		//reqcount--;
	}
	//outputfile.close();
	request_file.close();
	read_data();
	vis_taxi[1].resize(MAX_TAXIS, 0);
	vis_taxi[0].resize(MAX_TAXIS, (int)requests.size() * 2);
}

bool prune(double deregret, double detour)
{
	if (singleresult.empty()) {
		return false;
	}
	result_ptr p = singleresult.begin();
	return (p->deregret + eps) > deregret && (p->detour) < detour + eps;
}

void insert_result(int pick_pos, int drop_pos, double add_served_dist)
{
	double cur_serve_reg = (double)cur_unserve_count / cur_request_count;
	double cur_revenue_reg = (double)cur_unserve_dist / (cur_unserve_dist + cur_serve_dist);
	double cur_regret = alpha * cur_serve_reg + (1 - alpha) * cur_revenue_reg;
	cur_serve_dist = cur_serve_dist + add_served_dist;
	cur_unserve_dist = cur_unserve_dist - request.dist_sd;
	cur_unserve_count--;
	cur_serve_count++;
	double serve_reg = (double)cur_unserve_count / cur_request_count;
	double revenue_reg = (double)cur_unserve_dist / (cur_unserve_dist + cur_serve_dist);
	double regret = alpha * serve_reg + (1 - alpha) * revenue_reg;
	double deregret = cur_regret - regret;

	if (prune(deregret, request.detour)) {
		cur_serve_dist = cur_serve_dist - add_served_dist;
		cur_unserve_dist = cur_unserve_dist + request.dist_sd;
		cur_unserve_count++;
		cur_serve_count--;
		return;
	}
	singleresult.clear();
	vector<node_t*> schedule = cur_taxi.root;
	schedule.insert(schedule.begin() + pick_pos, request_node.first);
	schedule.insert(schedule.begin() + drop_pos, request_node.second);
	singleresult.insert(result_t(regret, serve_reg, revenue_reg,
		deregret, cur_serve_dist, cur_unserve_dist,
		cur_serve_count, cur_unserve_count,
		add_served_dist, request.detour, cur_taxi.id, schedule,
		pick_pos, drop_pos));
	cur_serve_dist = cur_serve_dist - add_served_dist;
	cur_unserve_dist = cur_unserve_dist + request.dist_sd;
	cur_unserve_count++;
	cur_serve_count--;
}