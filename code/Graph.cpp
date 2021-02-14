#include "Graph.h"
#include "main.h"

struct LabelComp
{
	bool operator() (const Label& a, const Label& b) const
	{
		return (a.distance < b.distance);
	}
}myLabelComp;

Graph::Graph(ifstream& in)
{
	readGraph(in);

	nodelist.resize(nr_node);
	for (int index = 0; index < nr_node; index++) {
		nodelist[index].rank = UINT_INFINITY;
		nodelist[index].flaga = false;
		nodelist[index].flagb = false;
	}

	inShortCutIndex = vector<int>(0);
	outShortCutIndex = vector<int>(0);

	outLabelList = vector<LabelList>(nr_node, LabelList(0));
	inLabelList = vector<LabelList>(nr_node, LabelList(0));

	pathIndex = vector<vector<int> >(nr_node, vector<int>(0));
	pathList = vector<vector<VertexID> >(nr_node, vector<EdgeID>(0));
}

void Graph::readGraph(ifstream& fs)
{
	// read the plain graph;
	fs >> nr_node;
	//cout << "# of all nodes: " << nr_node << endl;
	//int nr_cnode;
	//fs >> nr_cnode;
	//cout << "# of connected nodes: " << nr_cnode << endl;
	vector<vector<pair<int, double>>> tmpGraph(nr_node);
	for (size_t i = 0; i < nr_node; ++i) {
	//for (size_t i = 0; i < nr_cnode; ++i) {
		//cout << "node number: " << i << endl;
		auto& t = tmpGraph[i];
		//int sid; 
		//fs >> sid;
		fs.ignore(100, ':');
		//auto& t = tmpGraph[sid];
		char c;
		int x;
		double d;
		string s_d;
		do {
			fs >> x >> s_d;
			//cout << x <<" "<< s_d << " ";
			fs.get(c);
			//remove self loop
			if (x == i) {
				continue;
			}
			d = atof(s_d.c_str());
			t.emplace_back(x, d);
		} while (c == ' ');
		//cout << endl;
	}
	fs.close();

	// remove the multiple edges
	for (auto& v : tmpGraph) {
		sort(v.begin(), v.end());
		auto p = unique(v.begin(), v.end(), [](pair<int, double> a, pair<int, double> b) {
			return a.first == b.first;
			});
		v.resize(p - v.begin());
	}

	for (int i = 0; i < nr_node; i++) {
		outedgeindex.push_back((int)outedgelist.size());
		for (auto e : tmpGraph[i]) {
			outedgelist.emplace_back(e.second, i, e.first);
		}
	}
	outedgeindex.push_back((int)outedgelist.size());

	// build inverse graph;
	decltype(tmpGraph) tmpInverseGraph(tmpGraph.size());
	for (unsigned i = 0; i < tmpGraph.size(); i++) {
		for (auto e : tmpGraph[i]) {
			tmpInverseGraph[e.first].emplace_back(i, e.second);
		}
	}

	// materialize inedgeindex and inedgelist;
	for (int i = 0; i < nr_node; i++) {
		inedgeindex.push_back((int)inedgelist.size());
		for (auto e : tmpInverseGraph[i]) {
			inedgelist.emplace_back(e.second, i, e.first);
		}
	}
	inedgeindex.push_back((int)inedgelist.size());
}

int Graph::out_degree(int src)
{
	return outedgeindex[src + 1] - outedgeindex[src];
}

int Graph::in_degree(int src)
{
	return inedgeindex[src + 1] - inedgeindex[src];
}

// get a specified vertex property
Vertex& Graph::operator[](const int vid)
{
	assert(vid >= 0 && vid < nr_node);
	return nodelist[vid];
}

void Graph::insertShortcut(vector<ShortCuts>& tmpinshortcut, vector<ShortCuts>& tmpoutshortcut)
{
	inShortCutIndex.push_back(0);
	for (unsigned i = 0; i < tmpinshortcut.size(); i++) {
		for (unsigned j = 0; j < tmpinshortcut[i].size(); j++) {
			inShortCutList.push_back(tmpinshortcut[i][j]);
		}
		//ShortCuts(0).swap(tmpinshortcut[i]);
		inShortCutIndex.push_back((int)inShortCutList.size());
	}

	outShortCutIndex.push_back(0);
	for (unsigned i = 0; i < tmpoutshortcut.size(); i++) {
		for (unsigned j = 0; j < tmpoutshortcut[i].size(); j++) {
			outShortCutList.push_back(tmpoutshortcut[i][j]);
		}
		//ShortCuts(0).swap(tmpoutshortcut[i]);
		outShortCutIndex.push_back((int)outShortCutList.size());
	}
}

void Graph::insertNodeList(VertexList& nodelist)
{
	VertexList(0).swap(this->nodelist);
	for (unsigned i = 0; i < nodelist.size(); i++) {
		(this->nodelist).push_back(nodelist[i]);
	}
}

LabelList& Graph::exportOutLabel(VertexID source)
{
	return outLabelList[source];
}
LabelList& Graph::exportInLabel(VertexID target)
{
	return inLabelList[target];
}

void Graph::insertLabel(ifstream& in)
{
	outLabelList.resize(vertices.size());
	for (auto& it : outLabelList) {
		size_t n;
		in.read((char*)&n, sizeof(n));
		it.resize(n);
		in.read((char*)&it[0], n * sizeof(Label));
	}
	return;
}

vector<vector<int> >& Graph::exportPathIndex(void)
{
	return pathIndex;
}

vector<vector<VertexID> >& Graph::exportPathList(void)
{
	return pathList;
}

void Graph::insertPathIndex(vector<vector<double> >& data)
{
	vector<vector<int> >(0).swap(pathIndex);

	pathIndex.resize(data.size());
	for (unsigned i = 0; i < data.size(); i++) {
		for (unsigned j = 1; j < data[i].size(); j++) {
			pathIndex[i].push_back(static_cast<int>(data[i][j]));
		}
	}
}

void Graph::insertPathList(vector<vector<double> >& data)
{
	vector<vector<VertexID> >(0).swap(pathList);

	pathList.resize(data.size());
	for (unsigned i = 0; i < data.size(); i++) {
		for (unsigned j = 1; j < data[i].size(); j++) {
			pathList[i].push_back(static_cast<int>(data[i][j]));
		}
	}
}

ShortCuts& Graph::exportinShortCutList()
{
	return inShortCutList;
}

ShortCuts& Graph::exportoutShortCutList()
{
	return outShortCutList;
}

vector<int>& Graph::exportinShortCutIndex()
{
	return inShortCutIndex;
}

vector<int>& Graph::exportoutShortCutIndex()
{
	return outShortCutIndex;
}




