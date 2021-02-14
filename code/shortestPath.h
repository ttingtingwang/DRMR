#ifndef SHORTESTPATH_H
#define SHORTESTPATH_H

#include "lru_cache.h"
#include "Contraction.h"
#include "PerformanceTimer.h"
#include "HD.h"
#include "graphlink.h"
#include "vertex.h"
#include <math.h>

struct distanceElement
{
	double distance;
	VertexID meet_node;
};

class ShortestPath
{
private:
	Graph* g;
	HD* h;
	vector<VertexID> id_map;

	typedef plb::LRUCacheH4<uint64_t, distanceElement> dcache;
	typedef plb::LRUCacheH4<uint64_t, vector<int> > pcache;

	dcache* distance_cache; //distance cache
	pcache* path_cache; //path cache

	PerformanceTimer total_timer;
	PerformanceTimer distance_timer;
	PerformanceTimer path_timer;

	long long distance_queries;
	long long path_queries;
	long long distance_misses;
	long long path_misses;

	double distance(vertex* a, vertex* b);

public:
	vector<vertex*>& vertices;
	ShortestPath(vector<vertex*>& cvertices, string data_dir);
	vector<int> shortestPath(int start, int end);
	double pathDistance(vertex* start, queue<vertex*> path); //finds the length of a path returned by shortestPath
	double shortestDistance(int start, int end);
};

void shortest_path_init(vector<vertex*>& vertices, string data_dir);
double get_dist(int x, int y);
vector<int> get_path(int x, int y);
double hsdist(double lat1, double lon1, double lat2, double lon2);

#endif
