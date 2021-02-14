#include "main.h"

//simulation parameters
int MAX_PASSENGERS = 4; //capacity
int MAX_TAXIS = 2000;
double SERVICE_CONST = 1.5; //flexible factor
double PICKUP_CONST = 600;
//int logicalTime = 12600;//start and end time for the simulation
//int logicalTimeLimit = 14400;//for the entire dataset, use 0 (0:00) to 86400 (24:00)
//int logicalTime = 68400;//
//int logicalTimeLimit = 70200;//
//int logicalTime = 52200;//
//int logicalTimeLimit = 54000;//
int logicalTime = 0;//
int logicalTimeLimit = 10;//
double speed = 10;
//string data_dir = "/home/lab/wtt/data/newyork";
string data_dir = "/home/lab/wtt/data/chengdu";
double limit = 1;
double alpha = 0.5;
int batch = 1;
bool flag = true;


//statistics
//int count_dist;
//int count_taxi;
//int count_share;
size_t count_edge;
int count_node;

//data structures
queue<request_t> requests;
vector<taxi_t> taxis;
vector<vertex*> vertices;
ShortestPath* shortestPath;

int main(int argc, char** argv)
{
	range = 100;
	hasher = range / 2;
	//extern int search_limit;
	//search_limit = int(range * range * limit);
	//sscanf_s(argv[1], "%d", &MAX_PASSENGERS);
	//sscanf_s(argv[2], "%d", &MAX_TAXIS);
	//sscanf_s(argv[3], "%lf", &SERVICE_CONST);
	//sscanf_s(argv[4], "%lf", &PICKUP_CONST);
	//sscanf_s(argv[5], "%d", &logicalTimeLimit);

	grid_dist.resize(range);
	for (auto& v1 : grid_dist) {
		v1.resize(range);
		for (auto& v2 : v1) {
			v2.resize(range);
			for (auto& v3 : v2) {
				v3.resize(range);
			}
		}
	}
	extern vector<vector<set<int>>> grid_border;
	grid_border.resize(range);
	for (auto& v : grid_border) {
		v.resize(range);
	}
	for (int i = 0; i < 2; ++i) {
		vis_grid[i].resize(range);
		for (auto& v1 : vis_grid[i]) {
			v1.resize(range);
		}
	}
	grid.resize(range);
	for (auto& v : grid) {
		v.resize(range);
	}

	PICKUP_CONST *= speed;
	cout << "----------------------------" << endl;
	cout << "MAX_PASSENGERS = " << MAX_PASSENGERS << endl;
	cout << "MAX_TAXIS = " << MAX_TAXIS << endl;
	cout << "SERVICE_CONST = " << SERVICE_CONST << endl;
	cout << "PICKUP_CONST = " << PICKUP_CONST << endl;
	cout << "LogicalTimeLimit = " << logicalTimeLimit << endl;

	shortest_path_init(vertices, data_dir);
	cout << "memory=" << double(grid_dist.capacity() * grid_dist.capacity() * grid_dist.capacity() * grid_dist.capacity() * sizeof(double)+ grid.capacity() * grid_dist.capacity() * sizeof(grid_t)+ grid_border.capacity() * grid_border.capacity() * sizeof(int))/1024/1024 << endl;
	read_request(requests);
	taxis = set_taxi(MAX_TAXIS);
    //greedy_simulate(requests, taxis);
	replace_simulate(requests, taxis);

	return 0;
}