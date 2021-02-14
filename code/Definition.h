#ifndef DEFINITION_H_
#define DEFINITION_H_

#include <iostream>
#include <limits>
#include <vector>
using namespace std;

#define UINT_INFINITY 1073741823       //1024*1024*1024-1
#define INT_INFINITY numeric_limits<int>::max()
#define FLT_INFINITY numeric_limits<float>::max()
#define DBL_INFINITY numeric_limits<double>::max()

typedef unsigned int VertexID;
typedef unsigned int EdgeID;
typedef unsigned int Rank;
typedef double Weight;

struct Edge
{
	Weight weight;
	VertexID source;
	VertexID target;
	bool flaga;    // forward label;
	bool flagb;
	bool flagc;    // forward label;
	bool flagd;
	Edge(Weight w = 0.0, VertexID s = 0, VertexID t = 0)
		:weight(w), source(s), target(t)
	{
		flaga = flagb = flagc = flagd = false;
	}
};
typedef vector<Edge> EdgeList;

struct Vertex
{
	unsigned int rank;
	VertexID id;
	bool flaga;
	bool flagb;
	bool flagc;
	bool flagd;

	Vertex(int r = UINT_INFINITY, VertexID i = -1) : rank(r), id(i)
	{
		flaga = flagb = flagc = flagd = false;
	};
};
typedef vector<Vertex> VertexList;

struct ShortCutEdge
{
	vector<VertexID> innerIDs; // store the intermediate edges;
	VertexID target;
	Weight weight;
	bool flaga;
	bool flagb;

	ShortCutEdge()
	{
		flaga = false;
		flagb = false;
	};
};
typedef vector<ShortCutEdge> ShortCuts;

struct Label
{
	double distance;
	VertexID id;
	Label(double distance = 0, VertexID id = 0)
		:distance(distance), id(id)
	{
	}
};
typedef vector<Label> LabelList;



#endif
