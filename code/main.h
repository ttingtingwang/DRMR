#ifndef _MAIN_H_
#define _MAIN_H_

#include "includes.h"
#include "taxi.h"
#include "request.h"
#include "shortestPath.h"
#include "PerformanceTimer.h"
#include "grid.h"

extern int MAX_THREAD;
extern int MAX_PASSENGERS;
extern double PICKUP_CONST;
extern double SERVICE_CONST;
extern int logicalTime;
extern int logicalTimeLimit;
extern string data_dir;
extern double speed;
extern int MAX_TAXIS;
extern double alpha;
extern int batch;
extern bool flag;

#endif // _MAIN_H_

