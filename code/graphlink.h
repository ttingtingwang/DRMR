#ifndef __GRAPHLINK_H__
#define __GRAPHLINK_H__

#include "Contraction.h"
#include "PerformanceTimer.h"
#include "Graph.h"
#include "HD.h"
#include <cstring>

#include <iostream>
#include <time.h>

using namespace std;

void readHierarchies(Graph& graph, string hierarchy_file);
void checkHierarchies(Graph& graph, string file_name, bool gp);

#endif
