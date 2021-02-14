#include "main.h"

void greedy_simulate(queue<request_t>& requests, vector<taxi_t>& taxis)
{
	long long all_match_time = 0;
	int all_update_time = 0;
	queue<request_t> cur_requests;

	int se = SERVICE_CONST * 10;
	int pi = PICKUP_CONST / 10;
	int al = alpha * 10;
	int bt = batch;

	ofstream outputfile;
	if (flag) {
		outputfile.open(data_dir + "/res_syn/res10_2g_"
			+ to_string(MAX_PASSENGERS) + "_"
			+ to_string(MAX_TAXIS) + "_"
			+ to_string(se) + "_"
			+ to_string(pi) + "_"
			+ to_string(al) + "_"
			+ to_string(bt)
			+ ".txt");
	}
	else {
		outputfile.open(data_dir + "/res_syn/res10_n_g_"
			+ to_string(MAX_PASSENGERS) + "_"
			+ to_string(MAX_TAXIS) + "_"
			+ to_string(se) + "_"
			+ to_string(pi) + "_"
			+ to_string(al) + "_"
			+ to_string(bt)
			+ ".txt");
	}
	outputfile << "rid " << "time " << "source " << "destination "
		<< "dist " << "detour " << "adddist " << "tid " << endl;

	while (logicalTime < logicalTimeLimit) {
		cout << "time: " << logicalTime << endl;
		while (!requests.empty()) {
			if (requests.front().time > logicalTime + batch - 1) {
				break;
			}
			request_t r = requests.front();
			requests.pop();
			cur_requests.push(r);
		}
		cur_request_count = cur_request_count + cur_requests.size();
		cur_unserve_count = cur_unserve_count + cur_requests.size();
		int cur_totaldist = 0;
		for (int i = 0; i < cur_requests.size(); ++i) {
			request_t r = cur_requests.front();
			cur_totaldist = cur_totaldist + r.dist_sd;
			cur_requests.push(r);
			cur_requests.pop();
		}
		cur_allreq_dist = cur_allreq_dist + cur_totaldist;
		cur_unserve_dist = cur_unserve_dist + cur_totaldist;
		while (!cur_requests.empty()) {
			all_time.resume();
			request = cur_requests.front();
			cur_requests.pop();
			singleresult.clear();
			request_t r = request;
			node_t* pick = new node_t(r.id, r.pickup, r.count, true, inf);
			node_t* drop = new node_t(r.id + 1, r.dropoff, -r.count, false, SERVICE_CONST * r.dist_sd);
			request_node = make_pair(pick, drop);
			vector<int> matable_taxis;
			match_time.resume();
			matable_taxis = filter_taxi();
			//cout << "have " << matable_taxis.size() << " taxis" << endl;
			for (int i = 0; i < matable_taxis.size(); ++i) {
				int taxiid = matable_taxis[i];
				cur_taxi = taxis[taxiid];
				request_insertion();
			}
			if (singleresult.size() != 0) {
				int taxiid = singleresult.begin()->taxiid;
				vector<node_t*> schedule = singleresult.begin()->schedule;
				cur_taxi = taxis[taxiid];
				update_time.resume();
				push(schedule);
				update_time.pause();
				taxis[taxiid] = cur_taxi;
				cur_detour = cur_detour + singleresult.begin()->detour;
				cur_serve_dist = cur_serve_dist + singleresult.begin()->add_serve_dist;
				cur_serve_shortest_dist = cur_serve_shortest_dist + request.dist_sd;
				cur_unserve_dist = cur_unserve_dist - r.dist_sd;
				cur_unserve_count--;
				cur_serve_count++;
				double cur_serve_reg = (double)cur_unserve_count / cur_request_count;
				double cur_revenue_reg = (double)cur_unserve_dist / (cur_unserve_dist + cur_serve_dist);
				double cur_regret = alpha * cur_serve_reg + (1 - alpha) * cur_revenue_reg;

				cout << "rid: " << r.id / 2 << " {" << endl;
				cout << "tid: " << taxiid << endl;
				cout << "current regret: " << cur_regret << endl;
				cout << "current served regret: " << cur_serve_reg << endl;
				cout << "current revenue regret: " << cur_revenue_reg << endl;
				cout << "alrady serve: " << cur_serve_count << endl;
				cout << "all requests: " << cur_request_count << endl;
				cout << "served rate: " << (double)cur_serve_count / cur_request_count << endl;
				cout << "served distance: " << cur_serve_dist << endl;
				//all_match_time = all_match_time + match_time.sum.count();
				outputfile << r.id / 2 << " " << r.time << " " << r.pickup << " " << r.dropoff << " " << r.dist_sd
					<< " " << result.begin()->detour << " " << result.begin()->add_serve_dist
					<< " " << taxiid << endl;
				//match_time.clear();
			}
			delete drop;
			delete pick;
			match_time.pause();
			all_time.pause();
		}
		for (int i = 0; i < MAX_TAXIS; ++i) {
			all_time.resume();
			cur_taxi = taxis[i];
			update_time1.resume();
			move();
			update_time1.pause();
			taxis[i] = cur_taxi;
			all_time.pause();
		}
		//logicalTime++;
		logicalTime = logicalTime + batch;
	}
	cout << "the shortest distance: " << cur_serve_shortest_dist << endl;
	cout << "the actual distance: " << cur_serve_dist << endl;
	cout << "the schedule distance: " << taxi_dist << endl;
	cout << "the unified cost: " << taxi_dist + cur_unserve_dist << endl;
	//cout << (double)update_time.sum.count() / 1000 << endl;

	double serve_reg = (double)cur_unserve_count / cur_request_count;
	double revenue_reg = (double)cur_unserve_dist / (cur_unserve_dist + cur_serve_dist);
	double regret = alpha * serve_reg + (1 - alpha) * revenue_reg;



	outputfile << "all requests: " << cur_request_count << endl;
	outputfile << "unserved requests: " << cur_unserve_count << endl;
	outputfile << "unserved distance: " << cur_unserve_dist << endl;
	outputfile << "regret: " << regret << endl;
	outputfile << "served rate regret: " << serve_reg << endl;
	outputfile << "revenue regret: " << revenue_reg << endl;
	outputfile << "served rate: " << (double)cur_serve_count / cur_request_count << endl;
	outputfile << "the shortest distance: " << cur_serve_shortest_dist << endl;
	outputfile << "the actual distance: " << cur_serve_dist << endl;
	outputfile << "the schedule distance: " << cur_detour << endl;
	outputfile << "revenue: " << (cur_serve_dist - cur_detour) << endl;
	outputfile << "the unified cost: " << cur_detour + cur_unserve_dist << endl;
	outputfile << "the schdule update time: " << (double)update_time.sum.count() / 1000 / (1800 / batch) << endl;
	outputfile << "the update time: " << (double)update_time1.sum.count() / 1000 / (1800 / batch) << endl;
	outputfile << "the matching time: " << (double)match_time.sum.count() / cur_request_count / 1000 << endl;
	outputfile << "the total time: " << (double)all_time.sum.count() / 1000 << endl;
	outputfile.close();
}