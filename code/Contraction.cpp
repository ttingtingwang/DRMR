#include "Contraction.h"

struct QueueComp
{
	bool operator()(const pair<double, VertexID>& a, const pair<double, VertexID>& b) const
	{
		return (a.first > b.first);
	}
};
struct myQueueComp
{
	bool operator()(const pair<int, VertexID>& a, const pair<int, VertexID>& b) const
	{
		return (a.first > b.first);
	}
};

Contraction::Contraction(Graph& g) :graph(g)
{
	VertexList(0).swap(nodelist);
	nr_node = graph.nr_node;
	for (int i = 0; i < nr_node; i++) {
		nodelist.push_back(graph[i]);
	}

	//tmpinshortcutlist.resize(nr_node);
	tmpoutshortcutlist.resize(nr_node);

	pathIndex = vector<vector<unsigned> >(nr_node, vector<unsigned>(0));
	pathList = vector<vector<VertexID> >(nr_node, vector<VertexID>(0));
}

Contraction::~Contraction() {}


/* this function tries to find alternative shortest paths between u and v with two conditions:
	1. without through nid;
	2. within the distance limit.
*/
bool Contraction::dodgingDijkstra(VertexID source, VertexID target, VertexID nid, double limit, double& real_distance)
{
	forall_nodes(graph, vid)
	{
		nodelist[vid].flaga = false;
	}

	//cout << "Test: source = " << source << ", target = " << target << "." << endl;
	vector<double> dist(nr_node);
	forall_nodes(graph, vid)
	{
		dist[vid] = DBL_INFINITY;
	}
	dist[source] = 0.0;

	vector<VertexID> father(nr_node, numeric_limits<VertexID>::max());

	priority_queue<pair<double, VertexID>, vector<pair<double, VertexID> >, QueueComp> Queue;
	Queue.push(make_pair(0.0, source));

	while (Queue.size() > 0) {
		double min_dist = Queue.top().first;
		VertexID vid = Queue.top().second;
		//cout << "vid = " << vid << ", min_dist = " << min_dist << endl;
		if (vid == target) break;
		Queue.pop();
		if (min_dist > dist[vid]) 	continue; 					// lazy update;
		else						nodelist[vid].flaga = true; // settle vertex vid;

		if (min_dist >= limit) break;

		forall_outneighbors(graph, vid, eit)
		{
			if (nodelist[eit->target].flaga) {/*cout << "$ ";*/ continue; } // settled;
			if (dist[eit->target] > min_dist + eit->weight) {
				dist[eit->target] = min_dist + eit->weight;
				Queue.push(make_pair(dist[eit->target], eit->target));
				father[eit->target] = vid; // for test;
			}
		}

		for (unsigned i = 0; i < tmpoutshortcutlist[vid].size(); i++) {
			int tmptarget = tmpoutshortcutlist[vid][i].target;
			if (nodelist[tmptarget].flaga) {/*cout << "$ ";*/ continue; }
			if (dist[tmptarget] > min_dist + tmpoutshortcutlist[vid][i].weight) {
				dist[tmptarget] = min_dist + tmpoutshortcutlist[vid][i].weight;
				Queue.push(make_pair(dist[tmptarget], tmptarget));
				father[tmptarget] = vid; // for test;
			}
		}
	}

	// check whether nid has been used or not;
	unsigned c = target;
	bool used = false;
	while (father[c] != (unsigned)-1) {
		c = father[c];
		if (c == nid) { used = true; break; }
	}
	real_distance = dist[target];
	if (used) return false; // if the middle node is used;


	forall_nodes(graph, vid)
	{
		nodelist[vid].flaga = false;
	}
	// for test;
#ifdef CONTRACTION_DEBUG
	vector<int> path;
	path.push_back(target);
	while (true) {
		int c = path.back();
		if (father[c] != -1) path.push_back(father[c]);
		else break;
	}
	cout << "path: ";
	for (int i = path.size() - 1; i >= 0; i--) {
		cout << path[i] << " ";
	}
	cout << endl;
	cout << "dist: " << dist[target] << ", limit: " << limit << endl;
#endif

	if (dist[target] <= limit) { /*cout << "T" << endl;*/ return true; }
	else { /*cout << "F" << endl;*/ return false; } // the middle node is not used; however their distance is larger than the limit;
}

/*  this function checks whether there is an edge between source and target;
*/
inline bool Contraction::checkDuplicate(VertexID source, VertexID target, Weight weight)
{
	bool isfound = false;

	forall_outneighbors(graph, source, eit)
	{
		if (eit->target == target) {
			eit->weight = weight;
			isfound = true;
			break;
		}
	}
	for (unsigned i = 0; i < tmpoutshortcutlist[source].size(); i++) {
		if (tmpoutshortcutlist[source][i].target == target) {
			tmpoutshortcutlist[source][i].weight = weight;
			isfound = true;
			break;
		}
	}

	if (!isfound) return isfound;

	forall_outneighbors(graph, target, eit)
	{
		if (eit->target == source) {
			eit->weight = weight;
			break;
		}
	}
	for (unsigned i = 0; i < tmpoutshortcutlist[target].size(); i++) {
		if (tmpoutshortcutlist[target][i].target == source) {
			tmpoutshortcutlist[target][i].weight = weight;
			break;
		}
	}

	return isfound;
}

/*
	add shortcuts;
*/
void Contraction::packShortcut(int nid, \
	vector<VertexID>& neighbors, \
	vector<double>& distance, \
	vector<vector<VertexID> >& inner_IDs, \
	vector<int>& matched_pair, \
	vector<double>& pair_distance)
{

#ifdef CONTRACTION_DEBUG
	cout << "adding shortcuts..." << endl;
#endif
	assert(matched_pair.size() % 2 == 0);
	for (unsigned i = 0; i < matched_pair.size(); i += 2) {
		int source = neighbors[matched_pair[i]];
		int target = neighbors[matched_pair[i + 1]];
#ifdef CONTRACTION_DEBUG
		cout << "source = " << source << ", target = " << target << endl;
#endif
		Weight weight = pair_distance[i / 2];

		bool isfound = checkDuplicate(source, target, weight); // if found, update its weight;
		if (isfound) {
#ifdef CONTRACTION_DEBUG
			cout << "duplicate found!" << endl;
#endif
			continue;
		}
		else {
#ifdef CONTRACTION_DEBUG
			cout << "no duplicates!" << endl;
#endif
			addShortCut(source, target, nid, weight, inner_IDs[matched_pair[i]], inner_IDs[matched_pair[i + 1]]);
		}
	}

	removeEdge(nid);
}

/*
	add short cut edges;
*/
void Contraction::addShortCut(VertexID source, \
	VertexID target, \
	VertexID nid, \
	Weight weight, \
	vector<VertexID>& inner_ids_source, \
	vector<VertexID>& inner_ids_target)
{
	if (nodelist[source].rank < UINT_INFINITY) cout << "Wrong-ranking" << endl;
	if (nodelist[target].rank < UINT_INFINITY) cout << "Wrong-ranking" << endl;


	vector<VertexID> source_to_target;
	for (int i = int(inner_ids_source.size() - 1); i >= 0; i--) {
		source_to_target.push_back(inner_ids_source[i]);
	}
	source_to_target.push_back(nid);
	for (int i = 0; int(i < inner_ids_target.size()); i++) {
		source_to_target.push_back(inner_ids_target[i]);
	}

	vector<VertexID> target_to_source;
	for (int i = (int)source_to_target.size() - 1; i >= 0; i--) {
		target_to_source.push_back(source_to_target[i]);
	}

	ShortCutEdge shortcut_edge;
	shortcut_edge.target = source;
	shortcut_edge.flaga = false;
	shortcut_edge.flagb = false;
	shortcut_edge.innerIDs = source_to_target;
	shortcut_edge.weight = weight;

#ifdef CONTRACTION_DEBUG
	cout << "print out the edges in the path:";
	cout << source;
	for (int i = 0; i < source_to_target.size(); i++) {
		cout << "->" << source_to_target[i];
	}
	cout << "->" << target << endl;
#endif

	//tmpinshortcutlist[target].push_back( shortcut_edge );
	shortcut_edge.innerIDs = target_to_source;
	tmpoutshortcutlist[target].push_back(shortcut_edge);

	shortcut_edge.target = target;
	//tmpinshortcutlist[source].push_back( shortcut_edge );
	shortcut_edge.innerIDs = source_to_target;
	tmpoutshortcutlist[source].push_back(shortcut_edge);
}

void Contraction::computeShortcuts(bool gp)
{vector<VertexID> neighbors;
	vector<double> distance;
	vector<vector<VertexID> > inner_IDs;
	vector<int> matched_pair;
	vector<double> pair_distance;

	// initial the priority_queue;
	cout << "Calculate the initial queue ..." << endl;
	priority_queue<pair<int, VertexID>, vector<pair<int, VertexID> >, myQueueComp> queue;
	forall_nodes(graph, nid)
	{
		cout << "nid: " << nid << endl;
		int edgedifference = computeEdgeDifference(nid, neighbors, distance, inner_IDs, matched_pair, pair_distance);
		queue.push(make_pair(edgedifference, nid));
	}
	cout << "Done." << endl;

	int current_rank = 0;
	while (queue.size() > 1) {
		VertexID nid = queue.top().second;
		queue.pop();
		int current_edge_difference = computeEdgeDifference(nid, neighbors, distance, inner_IDs, matched_pair, pair_distance);
		if (current_edge_difference <= queue.top().first) {
			packShortcut(nid, neighbors, distance, inner_IDs, matched_pair, pair_distance);
			nodelist[nid].rank = current_rank++;
			cout << "queue size = " << queue.size() << " node = " << nid << endl;
		}
		else {
			queue.push(make_pair(current_edge_difference, nid));
		}
		//cout << nodelist[56402].rank << endl;
	}
	if (nodelist[queue.top().second].rank == UINT_INFINITY) nodelist[queue.top().second].rank = current_rank;

	// two things are produced: the shortcut edges, and the node rank;
	cout << "insert shortcut ..." << endl;
	graph.insertShortcut(tmpinshortcutlist, tmpoutshortcutlist);
	cout << "done." << endl;
	for (int i = 0; i < nr_node; i++) {
		graph[i].rank = nodelist[i].rank;
		graph[i].id = i;
	}

	// create labels;
	cout << "insert label ..." << endl;
	generateLabel(gp);
	cout << "done." << endl;
}

/* remove edges around the node;
*/
void Contraction::removeEdge(unsigned nid)
{
	// remove the edges in original graphs;
	forall_outneighbors(graph, nid, eit)
	{
		eit->flaga = true;
		forall_outneighbors(graph, eit->target, it)
		{
			if (it->target == nid) {
				it->flaga = true;
				//break;
			}
		}
	}
	for (unsigned i = 0; i < tmpoutshortcutlist[nid].size(); i++) {
		tmpoutshortcutlist[nid][i].flaga = true;
		int vid = tmpoutshortcutlist[nid][i].target;
		for (unsigned j = 0; j < tmpoutshortcutlist[vid].size(); j++) {
			if (tmpoutshortcutlist[vid][j].target == nid) {
				tmpoutshortcutlist[vid][j].flaga = true;
				//break;
			}
		}
	}
}

/*  this function computes the edge difference for vertex nid;
*/
int Contraction::computeEdgeDifference(int nid, \
	vector<VertexID>& neighbors, \
	vector<double>& distance, \
	vector<vector<VertexID> >& inner_IDs, \
	vector<int>& matched_pair, \
	vector<double>& pair_distance)
{
	vector<VertexID>(0).swap(neighbors);
	vector<double>(0).swap(distance);
	vector<vector<VertexID> >(0).swap(inner_IDs);
	vector<int>(0).swap(matched_pair);
	vector<double>(0).swap(pair_distance);

	// deal with neighbors in original graph;
	forall_outneighbors(graph, nid, eit)
	{
		if (eit->flaga) continue; // removed edges;
		if (nodelist[eit->target].rank < UINT_INFINITY) {
			cout << "Should not be dealed with! 1 " << eit->target << ' ' << nid << endl;
			exit(0);
			//break;
		}
		neighbors.push_back(eit->target);
		distance.push_back(eit->weight);
		// inner_IDs.push_back(vector<EdgeID>(1, graph.get_outedgeindex(nid, eit->target)));
		inner_IDs.push_back(vector<VertexID>(0));
	}

	// deal with neighbors linked with shortcuts;
	// here we deal with undirected graph;
	for (unsigned i = 0; i < tmpoutshortcutlist[nid].size(); i++) {
		if (tmpoutshortcutlist[nid][i].flaga) continue;
		if (nodelist[tmpoutshortcutlist[nid][i].target].rank < UINT_INFINITY) {
			cout << "Should not be dealed with! 2 " << tmpoutshortcutlist[nid][i].target << endl;
			exit(0);
		}
		neighbors.push_back(tmpoutshortcutlist[nid][i].target);
		distance.push_back(tmpoutshortcutlist[nid][i].weight);
		inner_IDs.push_back(tmpoutshortcutlist[nid][i].innerIDs);
	}

	// for (int i = 0; i < neighbors.size(); i++) cout << neighbors[i] << " ";
	// cout << endl;

	// calculate edge difference pairwisely;
	// this part could be expensive when average degree is high;
	int edgedifference = 0;
	for (unsigned i = 0; i < neighbors.size(); ++i) {
		for (unsigned j = i + 1; j < neighbors.size(); ++j) {
			if (neighbors[i] == neighbors[j]) continue;
			double real_distance = DBL_INFINITY;
			double limit = distance[i] + distance[j];
			bool isfound = dodgingDijkstra(neighbors[i], neighbors[j], nid, limit, real_distance);

			if (isfound) {
				continue;
			}
			edgedifference++;
			matched_pair.push_back(i);
			matched_pair.push_back(j);
			pair_distance.push_back(min(real_distance, limit));
		}
	}

	edgedifference = edgedifference - (int)neighbors.size();

	return edgedifference;
}

void Contraction::storePathInfo(int source, unsigned target, vector<VertexID>& father, vector<vector<VertexID> >& pathInfo)
{
	if (pathInfo[target].size() != 0) return;
	cout << "source: " << source << ", target: " << target << endl;

	if (father[source] != (unsigned)-1 && pathInfo[source].size() == 0) {
		int f = father[source];
		storePathInfo(f, source, father, pathInfo);
	}

	for (unsigned i = 0; i < pathInfo[source].size(); i++) {
		// cout << pathInfo[source][i] << " ";
		pathInfo[target].push_back(pathInfo[source][i]);
	}

	bool isfound = false;
	forall_outneighbors(graph, source, eit)
	{
		if (eit->target == target) {
			// cout << "direct!" << endl;
			isfound = true;
			pathInfo[target].push_back(target);
			break;

			cout << "target (original): " << target << ": ";
			for (unsigned i = 0; i < pathInfo[target].size(); i++) {
				cout << pathInfo[target][i] << " ";
			}
			cout << endl;
		}
	}
	if (isfound) return;
	// check the short cut graph;
	// if they are linked by the shortcut,
	// the inner vertex ids and the target node are added to the path;
	forall_outshortcuts(graph, source, eit)
	{
		if (eit->target == target) {
			// cout << "cut!" << endl;
			for (unsigned i = 0; i < eit->innerIDs.size(); i++) {
				pathInfo[target].push_back(eit->innerIDs[i]);
			}
			pathInfo[target].push_back(target);

			cout << "target (shortcut): " << target << ": ";
			for (unsigned i = 0; i < pathInfo[target].size(); i++) {
				cout << pathInfo[target][i] << " ";
			}
			cout << endl;

			return;
		}
	}
}

/* @parameter:
		father: record the predecesor of each node on the short path;
		index_list: record the shortest path end points for each node on the path;
		path_stack: record the current shortest path;
		new_path: when function returns, true; otherwise, false;
*/
void Contraction::buildPathIndex(VertexID source, vector<VertexID>& father, \
	vector<vector<unsigned> >& index_list, \
	vector<VertexID>& path_stack, \
	bool& new_path)
{
	// cout << "deal with " << path_stack.back() << endl;
	// cout << "path length " << path_stack.size() << endl;
	// dfs from source to find pathes;
	// a new node is appended;
	VertexID vid = path_stack.back();
	forall_outneighbors(graph, vid, eit)
	{
		VertexID uid = eit->target;
		if (father[uid] != vid) continue;

		// cout << "child: " << uid << endl;

		if (!new_path) {
			index_list[0][uid] = index_list[0][vid];
			index_list[1][uid] = index_list[1][vid] + 1;
			path_stack.push_back(uid);
			pathList[source].push_back(uid);
		}
		else {
			new_path = false;
			path_stack.push_back(uid);
			index_list[0][uid] = (int)pathList[source].size();
			for (unsigned i = index_list[0][vid]; i < index_list[1][vid]; i++) {
				pathList[source].push_back(pathList[source][i]);
			}
			pathList[source].push_back(uid);
			index_list[1][uid] = (int)pathList[source].size();
		}

		buildPathIndex(source, father, index_list, path_stack, new_path);
		path_stack.pop_back();
	}

	forall_outshortcuts(graph, vid, eit)
	{
		VertexID uid = eit->target;
		if (father[uid] != vid) continue;

		// cout << "short child: " << uid << endl;

		if (!new_path) {
			// cout << "old?" << endl;
			index_list[0][uid] = index_list[0][vid];
			index_list[1][uid] = index_list[1][vid] + (int)eit->innerIDs.size() + 1;
			for (unsigned i = 0; i < eit->innerIDs.size(); i++) {
				pathList[source].push_back(eit->innerIDs[i]);
			}
			path_stack.push_back(uid);
			pathList[source].push_back(uid);
		}
		else {
			// cout << "b " << index_list[0][vid] << ", e " << index_list[1][vid] << endl;
			index_list[0][uid] = (int)pathList[source].size();
			// cout << "new!" << endl;
			// cout << "vid " << vid << endl;
			new_path = false;
			path_stack.push_back(uid);
			index_list[0][uid] = (int)pathList[source].size();
			for (unsigned i = index_list[0][vid]; i < index_list[1][vid]; i++) {
				pathList[source].push_back(pathList[source][i]);
			}
			for (unsigned i = 0; i < eit->innerIDs.size(); i++) {
				pathList[source].push_back(eit->innerIDs[i]);
			}
			pathList[source].push_back(uid);
			index_list[1][uid] = (int)pathList[source].size();
		}

		buildPathIndex(source, father, index_list, path_stack, new_path);
		path_stack.pop_back();
	}

	new_path = true;
}

// create label for each vertex by Dijkstra algorithm on extended graphs;
void Contraction::createLabel(VertexID source, bool gp)
{
	//cout << "Test: source = " << source << endl;
	vector<double> dist(nr_node);
	for (int vid = 0; vid < nr_node; vid++) dist[vid] = DBL_INFINITY;
	dist[source] = 0.0;

	vector<VertexID> father(nr_node, numeric_limits<VertexID>::max());

	priority_queue<pair<double, VertexID>, vector<pair<double, VertexID> >, QueueComp> Queue;
	Queue.push(make_pair(0.0, source));

	for (int vid = 0; vid < nr_node; vid++) nodelist[vid].flaga = false;

	while (Queue.size() > 0) {
		double min_dist = Queue.top().first;
		VertexID vid = Queue.top().second;
		//cout << "vid = " << vid << ", min_dist = " << min_dist << endl;

		Queue.pop();
		if (min_dist > dist[vid]) {/*cout << "Greater!?" << endl;*/ continue; } // lazy update;
		else nodelist[vid].flaga = true; // settle vertex vid;

		forall_outneighbors(graph, vid, eit)
		{
			if (nodelist[eit->target].flaga) continue; // settled;
			if (/*vid != source &&*/ nodelist[eit->target].rank < nodelist[vid].rank) continue;
			if (dist[eit->target] > min_dist + eit->weight) {
				dist[eit->target] = min_dist + eit->weight;
				Queue.push(make_pair(dist[eit->target], eit->target));
				father[eit->target] = vid;
			}
		}

		forall_outshortcuts(graph, vid, eit)
		{
			if (nodelist[eit->target].flaga) continue; // settled;
			if (/*vid != source &&*/ nodelist[eit->target].rank < nodelist[vid].rank) continue;
			double found = false;
			for (unsigned j = 0; j < eit->innerIDs.size(); j++) {
				VertexID iv = eit->innerIDs[j];
				// if (dist[iv] >= dist[eit->target]) {found = true; break;}
				if (dist[iv] <= dist[vid]) { found = true; break; }
			}
			if (found) continue;
			if (dist[eit->target] > min_dist + eit->weight) {
				dist[eit->target] = min_dist + eit->weight;
				Queue.push(make_pair(dist[eit->target], eit->target));
				father[eit->target] = vid;
			}
		}
	}

	if (/*inlabellist.size() < nr_node ||*/ outlabellist.size() < nr_node) {
		//inlabellist.resize(nr_node);
		outlabellist.resize(nr_node);
	}

	// add in inlabellist and outlabellist;
	for (int vid = 0; vid < nr_node; vid++) {
		if (dist[vid] < DBL_INFINITY) {
			Label tmp_label;
			tmp_label.id = vid;
			tmp_label.distance = dist[vid];
			outlabellist[source].push_back(tmp_label);
			//tmp_label.id = source;
			//inlabellist[vid].push_back(tmp_label);
		}
	}

	if (!gp) return;

	// create path information for unpacking path;
	vector<vector<unsigned> > index_list(2, vector<unsigned>(nr_node, 0));
	vector<VertexID> path_stack;
	bool new_path = false;
	path_stack.push_back(source);
	index_list[0][source] = 0; // beginning of the path;
	index_list[1][source] = 1; // end of the path + 1;
	pathList[source].push_back(source);
	buildPathIndex(source, father, index_list, path_stack, new_path);

	// create path index for each node;
	for (int vid = 0; vid < nr_node; vid++) {
		if (dist[vid] < DBL_INFINITY) {
			pathIndex[source].push_back(index_list[0][vid]);
			pathIndex[source].push_back(index_list[1][vid]);
		}
	}
}

void Contraction::generateLabel(bool gp)
{
	for (int i = 0; i < nr_node; i++) {
		if (i % 1000 == 0) {
			cout << "generate label: " << i << endl;
		}
		createLabel(i, gp);
	}
	graph.insertShortcut(tmpinshortcutlist, tmpoutshortcutlist);
}

vector<ShortCuts>& Contraction::exportOutShortcut(void)
{
	return tmpoutshortcutlist;
}

vector<ShortCuts>& Contraction::exportInShortcut(void)
{
	return tmpinshortcutlist;
}

VertexList& Contraction::exportNodeList(void)
{
	return nodelist;
}

LabelList& Contraction::exportOutLabel(VertexID source)
{
	return outlabellist[source];
}

LabelList& Contraction::exportInLabel(VertexID source)
{
	return inlabellist[source];
}

void Contraction::printLabel()
{
	cout << "out label list: " << endl;
	for (unsigned i = 0; i < outlabellist.size(); i++) {
		cout << i << ": ";
		for (unsigned j = 0; j < outlabellist[i].size(); j++) {
			cout << "[" << outlabellist[i][j].id << ", " << outlabellist[i][j].distance << "] ";
		}
		cout << endl;
	}
}

void Contraction::printPath()
{
	cout << "print path information:" << endl;
	for (unsigned i = 0; i < pathIndex.size(); i++) {
		cout << i << ":";
		for (unsigned j = 0; j < pathIndex[i].size() - 1; j++) {
			int begin = pathIndex[i][j];
			int end = pathIndex[i][j + 1];
			cout << outlabellist[i][j].id << "[";
			for (; begin < end; begin++) {
				cout << pathList[i][begin] << " ";
			}
			cout << "] ";
		}
		cout << endl;
	}
}
