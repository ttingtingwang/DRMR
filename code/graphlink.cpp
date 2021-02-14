#include "main.h"
#include "graphlink.h"

void readHierarchies(Graph& graph, string hierarchy_file)
{
	vector<ShortCuts> outshortcut, inshortcut;
	VertexList nodelist;
	auto nr_node = graph.nr_node;
	// outshortcutlist;
	size_t found = hierarchy_file.find_last_of(".");
	hierarchy_file.erase(hierarchy_file.begin() + found, hierarchy_file.end());
	hierarchy_file += ".hierarchy_out";
	ifstream in(hierarchy_file.c_str(), ios::in | ios::binary);
	size_t n;
	in.read((char*)&n, sizeof(n));
	outshortcut.resize(vertices.size());
	for (int i = 0; i < n; ++i) {
		size_t x;
		in.read((char*)&x, sizeof(x));
		outshortcut[x].emplace_back();
		ShortCutEdge& e = outshortcut[x].back();
		in.read((char*)&e.target, sizeof(VertexID));
		in.read((char*)&e.weight, sizeof(Weight));
		size_t m;
		in.read((char*)&m, sizeof(m));
		e.innerIDs.resize(m);
		in.read((char*)&e.innerIDs[0], m * sizeof(VertexID));
	}
	in.close();

	graph.insertShortcut(inshortcut, outshortcut);

	// nodelist;
	found = hierarchy_file.find_last_of("_");
	hierarchy_file.erase(hierarchy_file.begin() + found, hierarchy_file.end());
	hierarchy_file += "_node";
	in.open(hierarchy_file.c_str(), ifstream::in);
	vector<unsigned> rank(nr_node);
	in.read((char*)&rank[0], nr_node * sizeof(unsigned));
	in.close();
	nodelist.resize(nr_node);
	for (int i = 0; i < nr_node; i++) {
		nodelist[i].id = i;
		nodelist[i].rank = rank[i];
	}

	graph.insertNodeList(nodelist);

	found = hierarchy_file.find_last_of(".");
	hierarchy_file.erase(hierarchy_file.begin() + found, hierarchy_file.end());
	hierarchy_file += ".label_out";
	in.open(hierarchy_file.c_str(), ios::binary);
	//readFile(in, data);
	graph.insertLabel(in);
	//cout << "vid = " << graph.exportOutLabel(19270)[0].id << ", dist = " << graph.exportOutLabel(19270)[0].distance << endl;
	in.close();
}

// check whether hierachies have been built or not;
// if yes, read them in; otherwise, build them;
void checkHierarchies(Graph& graph, string file_name, bool gp)
{
	string hierarchy_file = file_name;
	size_t found = hierarchy_file.find_last_of('.');
	hierarchy_file.erase(hierarchy_file.begin() + found, hierarchy_file.end());
	hierarchy_file += ".hierarchy_out";

	ifstream infile(hierarchy_file.c_str());
	if (infile) {
		readHierarchies(graph, hierarchy_file);
		cout << "Finished reading hierarchy ... " << endl;
		return;
	}
	cout << "Creating hierarchy ... " << endl;
	Contraction c(graph);
	c.computeShortcuts(gp);

	const vector<ShortCuts>& outshortcut = c.exportOutShortcut();
	const VertexList& nodelist = c.exportNodeList();
	ofstream out(hierarchy_file.c_str(), ios::binary);
	auto n = outshortcut.size();
	out.write((char*)&n, sizeof(n));
	for (size_t i = 0; i < n; ++i) {
		for (auto edge : outshortcut[i]) {
			out.write((char*)&i, sizeof(i));
			out.write((char*)&edge.target, sizeof(VertexID));
			out.write((char*)&edge.weight, sizeof(Weight));
			size_t m = edge.innerIDs.size();
			out.write((char*)&m, sizeof(m));
			out.write((char*)&edge.innerIDs[0], m * sizeof(VertexID));
		}
	}
	out.close();

	found = hierarchy_file.find_last_of("_");
	hierarchy_file.erase(hierarchy_file.begin() + found, hierarchy_file.end());
	hierarchy_file += "_node";
	out.open(hierarchy_file.c_str(), ofstream::out);
	n = nodelist.size();
	vector<unsigned> rank(n);
	for (unsigned i = 0; i < n; i++) {
		rank[i] = nodelist[i].rank;
	}
	out.write((char*)&rank[0], n * sizeof(unsigned));
	out.close();

	found = hierarchy_file.find_last_of(".");
	hierarchy_file.erase(hierarchy_file.begin() + found, hierarchy_file.end());
	hierarchy_file += ".label_out";
	out.open(hierarchy_file.c_str(), ios::binary);
	for (unsigned i = 0; i < nodelist.size(); i++) {
		LabelList& label = c.exportOutLabel(i);
		auto n = label.size();
		out.write((char*)&n, sizeof(n));
		out.write((char*)&label[0], label.size() * sizeof(Label));
	}
	out.close();
}
