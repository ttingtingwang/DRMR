#include "main.h"


int hasher, range;
vertex_t Node[62000]; // newyork:62000 chengdu: 37000
//vertex_t Node[37000];
vector<vector<grid_t>> grid;
vector<vector<vector<vector<double>>>> grid_dist;
set<grid_t*> need_update;

double get_dist_lx(const edge_t x)
{
	return x.first->parent->dist_root + x.second->dist;
}

bool cmp_lx(const edge_t x, const edge_t y)
{
	return get_dist_lx(x) < get_dist_lx(y);
}

bool cmp_passenger(const edge_t x, const edge_t y)
{
	return x.first->passenger < y.first->passenger;
}

bool cmp_dist(const edge_t x, const edge_t y)
{
	return x.first->dist_parent < y.first->dist_parent;
}

bool cmp_slack(const edge_t x, const edge_t y)
{
	return x.first->dist_slack < y.first->dist_slack;
}

void grid_t::update_tree_edge(edge_t edge, opcode op)
{
	if (op == INSERT) {
		tree_edges.insert(edge);
	}
	else {
		tree_edges.erase(edge);
	}
	need_update.insert(this);
}

void grid_t::update_empty_edge(edge_t edge, opcode op)
{
	if (op == INSERT) {
		empty_edges.insert(edge);
	}
	else {
		if (empty_edges.find(edge) != empty_edges.end()) {
			empty_edges.erase(edge);
		}
	}
}

void grid_t::update()
{
	if (tree_edges.empty()) {
		return;
	}
	max_passenger = MAX_PASSENGERS - min_element(tree_edges.begin(), tree_edges.end(), cmp_passenger)->first->passenger;
	max_dist = max_element(tree_edges.begin(), tree_edges.end(), cmp_dist)->first->dist_parent;
	//min_lx = get_dist_lx(*min_element(tree_edges.begin(), tree_edges.end(), cmp_lx));
	max_slack = max_element(tree_edges.begin(), tree_edges.end(), cmp_slack)->first->dist_slack;
}

extern size_t count_edge;
void update_grid()
{
	size_t sum = 0;
	for (int i = 0; i < range; ++i) {
		for (int j = 0; j < range; ++j) {
			sum += grid[i][j].empty_edges.size() + grid[i][j].tree_edges.size();
		}
	}
	count_edge = max(count_edge, sum);

	sum = 0;
	for (int i = 0; i < MAX_TAXIS; i++) {
		//sum += taxis[i].root.count_nodes();
	}
	count_node = max(count_node, (int)sum);


	for (auto g : need_update) {
		g->update();
	}
	need_update.clear();
}

double ldist(int s, grid_id g)
{
	grid_id gs = Node[s].grid;
	return gs == g ? 0 :
		grid_dist[gs.first][gs.second][g.first][g.second]
		+ Node[s].min_dis;
}

double ldist(int s, int d)
{
	grid_id gs = Node[s].grid, gd = Node[d].grid;
	return gs == gd ? 0 :
		grid_dist[gs.first][gs.second][gd.first][gd.second]
		+ Node[s].min_dis + Node[d].min_dis;
}

//extern int count_dist;
grid_id get_hash(int i)
{
	int hash_x = (((int)((vertices[i]->x) * 1000000)) % HASHINGLONG) / (1000000 / hasher);
	hash_x = min(hash_x, range - 1);
	hash_x = max(hash_x, 0);
	int hash_y = (((int)((vertices[i]->y) * 1000000)) % HASHINGLAT) / (1000000 / hasher);
	hash_y = min(hash_y, range - 1);
	hash_y = max(hash_y, 0);
	return make_pair(hash_x, hash_y);
}

//set<int> grid_border[RANGE][RANGE];
vector<vector<set<int>>> grid_border;

void prepare()
{
	//calc grid
	for (unsigned i = 0; i < vertices.size(); i++) {
		if (i == 607) {
			int f = 0;
		}
		if (i==50506) {
			int f = 0;
		}
		Node[i].grid = get_hash(i);
	}

	//calc border
	ifstream edgeFile;
	int nr_edge;
	edgeFile.open((data_dir + "/edges.out").c_str());
	edgeFile >> nr_edge;
	for (int i = 0; i < nr_edge; i++) {
		int a, b;
		double distance;
		edgeFile >> a >> b >> distance;
		if (Node[a].grid != Node[b].grid) {
			grid_id ga = Node[a].grid;
			grid_id gb = Node[b].grid;
			grid_border[ga.first][ga.second].insert(b);
			grid_border[gb.first][gb.second].insert(a);
		}
	}
	edgeFile.close();

	//calc grid_dist
	long long max_hash = (long long)range * range * range * range;
	for (long long hash = 0; hash < max_hash; ++hash) {
		int i = hash % range;
		int j = hash / range % range;
		int u = hash / range / range % range;
		int v = hash / range / range / range % range;
		if (hash % (range * range) == 0) {
			cout << u << ' ' << v << endl;
		}
		if (i == u && j == v) {
			continue;
		}
		else if (grid_dist[i][j][u][v] > 0) {
			continue;
		}
		double &min_dist = grid_dist[i][j][u][v];
		min_dist = 1e8;
		for (auto x : grid_border[i][j]) {
			for (auto y : grid_border[u][v]) {
				min_dist = min(min_dist, get_dist(x, y));
				//count_dist--;
			}
		}
		grid_dist[u][v][i][j] = min_dist;
	}

	//calc Node[i].dist
	for (unsigned i = 0; i < vertices.size(); i++) {
		for (auto x : grid_border[Node[i].grid.first][Node[i].grid.second]) {
			Node[i].min_dis = min(Node[i].min_dis, get_dist(i, x));
			//count_dist--;
		}
	}

	//write file
	ofstream fp;
	char s[100];
	sprintf(s, "%s/grid_dist_%d.dat", data_dir.c_str(), range);
	fp.open(s, ios::binary);
	//fp.write((char*)grid_dist, RANGE*RANGE*RANGE*RANGE * sizeof(double));
	for (auto v1 : grid_dist) {
		for (auto v2 : v1) {
			for (auto v3 : v2) {
				for (auto v4 : v3) {
					fp.write((char*)&v4, sizeof(double));
				}
			}
		}
	}
	fp.close();
	sprintf(s, "%s/node_%d.dat", data_dir.c_str(), range);
	fp.open(s, ios::binary);
	fp.write((char*)Node, vertices.size() * sizeof(vertex_t));
	fp.close();
}

void read_data()
{
	ifstream fp;
	char s[100];
	sprintf(s, "%s/grid_dist_%d.dat", data_dir.c_str(), range);
	fp.open(s, ios::binary);
	if (!fp) {
		fp.close();
		prepare();
		fp.open(s, ios::binary);
	}
	for (auto v1 : grid_dist) {
		for (auto v2 : v1) {
			for (auto v3 : v2) {
				for (auto v4 : v3) {
					fp.read((char*)&v4, sizeof(double));
				}
			}
		}
	}
	fp.close();

	sprintf(s, "%s/node_%d.dat", data_dir.c_str(), range);
	fp.open(s, ios::binary);
	fp.read((char*)Node, vertices.size() * sizeof(vertex_t));
	fp.close();
}