#ifndef CONTRACTION_H_
#define CONTRACTION_H_

#include <iostream>
#include <vector>
#include <queue>

#include "Graph.h"
#include "Config.h"

using namespace std;

//#define CONTRACTION_DEBUG

class Contraction
{

private:
	Graph& graph;
	VertexList nodelist;
	int nr_node;

	vector<ShortCuts> tmpinshortcutlist; // for preprocessing;
	vector<ShortCuts> tmpoutshortcutlist;

	vector<LabelList> outlabellist;
	vector<LabelList> inlabellist;

	vector<vector<unsigned> > pathIndex;
	vector<vector<VertexID> > pathList;

public:
	Contraction(Graph&);
	virtual ~Contraction(void);

	bool dodgingDijkstra(VertexID, VertexID, VertexID, double, double&);
	inline bool checkDuplicate(VertexID, VertexID, Weight);
	void removeEdge(unsigned);
	void packShortcut(int, vector<VertexID>&, vector<double>&, vector<vector<EdgeID> >&, vector<int>&, vector<double>&);
	void addShortCut(VertexID, VertexID, VertexID, Weight, vector<EdgeID>&, vector<EdgeID>&);
	int  computeEdgeDifference(int, vector<VertexID>&, vector<double>&, vector<vector<EdgeID> >&, vector<int>&, vector<double>&);
	void computeShortcuts(bool);

	vector<ShortCuts>& exportOutShortcut(void);
	vector<ShortCuts>& exportInShortcut(void);
	VertexList& exportNodeList(void);
	LabelList& exportOutLabel(VertexID);
	LabelList& exportInLabel(VertexID);

	void createLabel(VertexID, bool);
	void generateLabel(bool);
	void printLabel(void);
	void printPath(void);

	void storePathInfo(int, unsigned, vector<VertexID>&, vector<vector<VertexID> >&);

	void buildPathIndex(VertexID, vector<VertexID>&, vector<vector<unsigned> >&, vector<VertexID>&, bool&);
};

#endif
