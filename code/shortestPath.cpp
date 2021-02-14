#include "shortestPath.h"
#include "Config.h"

#define PI 3.14159265

double ShortestPath::distance(vertex* a, vertex* b)
{
	double d1 = a->x - b->x;
	double d2 = a->y - b->y;
	return sqrt(d1 * d1 + d2 * d2);
}

ShortestPath::ShortestPath(vector<vertex*>& cvertices, string data_dir) : vertices(cvertices)
{
	distance_cache = new dcache(100);
	path_cache = new pcache(200);

	//for statistics
	distance_queries = 0;
	path_queries = 0;
	distance_misses = 0;
	path_misses = 0;
	//total_timer.resume();

	string filename = data_dir + "/edges.gra";

	cout << "ShortestPath: opening " << filename << endl;
	ifstream infile(filename.c_str());
	if (!infile) {
		cout << "ShortestPath: Error: Cannot open " << filename << endl;
		exit(0);
	}
	g = new Graph(infile);
	cout << "Checking hierarchy ... " << endl;
	checkHierarchies(*g, filename, false);

	h = new HD(*g); // highway dimension method;

	cout << "ShortestPath: finished loading" << endl;
}

vector<int> ShortestPath::shortestPath(int start, int end)
{
	if (start == end) {
		vector<int> equeue;
		equeue.push_back(start);
		return equeue;
	}
	path_queries++;

	//search cache first
	uint64_t x = ((uint64_t)start) * vertices.size() + (uint64_t)end;
	pcache::const_iterator it = path_cache->find(x);

	if (it != path_cache->end()) {
		return it.value();
	}
	path_misses++;
	dcache::const_iterator d_it = distance_cache->find(x);

	if (d_it == distance_cache->end()) {
		//well, distance wasn't there
		//call shortestDistance so it calculates it, and retrieve from cache
		// (since we need the meet node, not just distance)
		shortestDistance(start, end);
		d_it = distance_cache->find(x);
	}

	distanceElement distance_el = d_it.value();

	//we execute findPath twice on closer points to minimize execution time
	vector<VertexID> path_from_source, path_to_target;
	h->findPath(start, distance_el.meet_node, path_from_source, distance_el.distance);
	h->findPath(end, distance_el.meet_node, path_to_target, distance_el.distance);

	//reconstruct the path from the subpaths

	vector<int> actualPath;
	actualPath.push_back(start);
	for (int i = (int)path_from_source.size() - 2; i >= 0; i--) {
		actualPath.push_back(path_from_source[i]);
	}

	for (unsigned i = 1; i < path_to_target.size(); i++) {
		actualPath.push_back(path_to_target[i]);
	}

	vector<int> queuecopy = actualPath;
	(*path_cache)[x] = queuecopy;

	return actualPath;
}

double ShortestPath::pathDistance(vertex* start, queue<vertex*> path)
{
	if (path.size() == 0) return 0;

	double totalDistance = distance(start, path.front());
	while (path.size() > 1) {
		vertex* popped = path.front();
		path.pop();

		totalDistance += distance(popped, path.front());
	}

	return totalDistance;
}

double ShortestPath::shortestDistance(int start, int end)
{
	distance_queries++;

	//find the unique identifier for this distance
	uint64_t x = ((uint64_t)start) * vertices.size() + (uint64_t)end;

	dcache::const_iterator it = distance_cache->find(x);

	if (it != distance_cache->end()) {
		return (*distance_cache)[x].distance;
	}
	distance_misses++;

	distanceElement distance_el;
	distance_el.meet_node = numeric_limits<VertexID>::max();
	distance_el.distance = h->run(start, end, distance_el.meet_node);
	(*distance_cache)[x] = distance_el;

	return distance_el.distance;
}

ShortestPath* shortest_path;

void read_graph(vector<vertex*>& vertices, string data_dir)
{
	ifstream vertexFile;
	int nr_vertex, nr_edge;
	vertexFile.open((data_dir + "/vertices.out").c_str());
	vertexFile >> nr_vertex;
	for (int i = 0; i < nr_vertex; i++) {
		vertex* newVertex = new vertex;
		newVertex->id = i;
		vertexFile >> newVertex->x >> newVertex->y;
		vertices.push_back(newVertex);
	}
	vertexFile.close();
    //cout << "node numbers: " << vertices.size() << "\n";
	cout << "Read Vertices: done" << endl;

	ifstream edgeFile;
	edgeFile.open((data_dir + "/edges.out").c_str());
	edgeFile >> nr_edge;
	for (int i = 0; i < nr_edge; i++) {
		int a, b;
		double distance;
		edgeFile >> a >> b >> distance;
		vertices[a]->neighbors.push_back(vertices[b]);
		vertices[b]->neighbors.push_back(vertices[a]);
	}
	edgeFile.close();
	cout << "Read Edges: done" << endl;
}

void shortest_path_init(vector<vertex*>& vertices, string data_dir)
{
	read_graph(vertices, data_dir);
	shortest_path = new ShortestPath(vertices, data_dir);
}

extern int count_dist;
double get_dist(int x, int y)
{
	//++count_dist;
	return shortest_path->shortestDistance(x, y);
}

vector<int> get_path(int x, int y)
{
	return shortest_path->shortestPath(x, y);
}

//calculation Haversine distance
double HaverSin(double theta)
{
	double v = sin(theta / 2);
	return v * v;
}

double ConvertDegreesToRadians(double degrees)
{
	return degrees * PI / 180;
}

static double EARTH_RADIUS = 6371.0;//earth radius/km

double hsdist(double lat1, double lon1, double lat2, double lon2)
{
	lat1 = ConvertDegreesToRadians(lat1);
	lon1 = ConvertDegreesToRadians(lon1);
	lat2 = ConvertDegreesToRadians(lat2);
	lon2 = ConvertDegreesToRadians(lon2);

	double vLon = abs(lon1 - lon2);
	double vLat = abs(lat1 - lat2);

	//h is the great circle distance in radians
	//double h = HaverSin(vLat) + cos(lat1) * cos(lat2) * HaverSin(vLon);

	//double distance = 2 * EARTH_RADIUS * asin(sqrt(h));

	double distance = 2 * asin(sqrt(pow(sin(vLat / 2), 2) + cos(lat1) * cos(lat2) * pow(sin(vLon / 2), 2)));
	distance = distance * EARTH_RADIUS;
	distance = round(distance * 10000) / 10000;

	return distance;
}

