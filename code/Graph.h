#ifndef _STATIC_GRAPH_H
#define _STATIC_GRAPH_H

#include "Definition.h"
#include "Config.h"
#include "PerformanceTimer.h"
#include <cstring>

//#define GRAPH_DEBUG 

class HD;

// unweighted and directed graph
struct Graph
{
	VertexList nodelist;
	EdgeList inedgelist, outedgelist;
	//
	//outedgelist: distance, vertexid, neighborid
	vector<int> inedgeindex, outedgeindex;
	//vector<degreetype> inportlist, outportlist; // entity(x,y) represents the port id of vertex x in the neighbor list of vertex y
	int nr_node;

	ShortCuts inShortCutList, outShortCutList;
	vector<int> inShortCutIndex, outShortCutIndex;

	vector<LabelList> outLabelList;
	vector<LabelList> inLabelList;

	vector<vector<int> > pathIndex;
	vector<vector<VertexID> > pathList;

	friend class HD;

	Graph(ifstream& gio);
	void readGraph(ifstream& gio);
	int in_degree(int nodeid);
	int out_degree(int nodeid);
	//void reorderid(const vector<int>& nodemap);
	Graph& operator=(const Graph& graph);
	Vertex& operator[](const int id);

	void insertShortcut(vector<ShortCuts>&, vector<ShortCuts>&);
	void insertNodeList(VertexList&);

	void insertLabel(ifstream& in);

	LabelList& exportOutLabel(VertexID);
	LabelList& exportInLabel(VertexID);

	vector<vector<int> >& exportPathIndex(void);
	vector<vector<VertexID> >& exportPathList(void);
	void insertPathIndex(vector<vector<double> >&);
	void insertPathList(vector<vector<double> >&);

	ShortCuts& exportinShortCutList();
	ShortCuts& exportoutShortCutList();
	vector<int>& exportinShortCutIndex();
	vector<int>& exportoutShortCutIndex();

	// utility functions

	inline vector<Edge>::iterator firstoutneighbor(const int vid)
	{
		return outedgelist.begin() + outedgeindex[vid];
	}
	inline vector<Edge>::iterator endofoutneighbor(const int vid)
	{
		return outedgelist.begin() + outedgeindex[vid + 1];
	}
	inline vector<Edge>::iterator firstinneighbor(const int vid)
	{
		return inedgelist.begin() + inedgeindex[vid];
	}
	inline vector<Edge>::iterator endofinneighbor(const int vid)
	{
		return inedgelist.begin() + inedgeindex[vid + 1];
	}

	inline ShortCuts::iterator firstoutShortCut(const int vid)
	{
		return outShortCutList.begin() + outShortCutIndex[vid];
	}
	inline ShortCuts::iterator endofoutShortCut(const int vid)
	{
		return outShortCutList.begin() + outShortCutIndex[vid + 1];
	}
	inline ShortCuts::iterator firstinShortCut(const int vid)
	{
		return inShortCutList.begin() + inShortCutIndex[vid];
	}
	inline ShortCuts::iterator endofinShortCut(const int vid)
	{
		return inShortCutList.begin() + inShortCutIndex[vid + 1];
	}
};

// traverse all nodes
#define forall_nodes(G,nid)  for(int nid=0; nid<G.nr_node; nid++)
// traverse all outgoing neighbor of specific node nid
#define forall_outneighbors(G,nid,outneighbor_ptr)  \
	for (vector<Edge>::iterator outneighbor_ptr=G.firstoutneighbor(nid); outneighbor_ptr!=G.endofoutneighbor(nid); outneighbor_ptr++)
// traverse all incoming neighbor of specific node nid
#define forall_inneighbors(G,nid,inneighbor_ptr)  \
	for (vector<Edge>::iterator inneighbor_ptr=G.firstinneighbor(nid); inneighbor_ptr!=G.endofinneighbor(nid); inneighbor_ptr++)	
// traverse all outgoing shortcuts of specific node nid;
#define forall_outshortcuts(G,nid,outshortcut_ptr)  \
	for (ShortCuts::iterator outshortcut_ptr=G.firstoutShortCut(nid); outshortcut_ptr!=G.endofoutShortCut(nid); outshortcut_ptr++)
// traverse all incoming neighbor of specific node nid
#define forall_inshortcuts(G,nid,inshortcut_ptr)  \
	for (ShortCuts::iterator inshortcut_ptr=G.firstinShortCut(nid); inshortcut_ptr!=G.endofinShortCut(nid); inshortcut_ptr++)

#endif
