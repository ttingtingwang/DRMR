#include "main.h"

queue<request_t> cur_reqs;
map<int, request_t> req_list;
map<int, map<int, taxi_t>> taxi_request;
map<int, pair<set<result_t>, set<result_t>>> request_remain_result;
map<int, map<grid_id, bool>> inavailable;
map<int, set<result_t>> request_taxi_pair;
ofstream outputfile;
int replacecount = 0;

void pair_request_taxi() {
	while (!cur_reqs.empty()) {
		request = cur_reqs.front();
		cur_reqs.pop();
		result.clear();
		request_t r = request;
		node_t* pick = new node_t(r.id, r.pickup, r.count, true, inf);
		node_t* drop = new node_t(r.id + 1, r.dropoff, -r.count, false, SERVICE_CONST * r.dist_sd);
		request_node = make_pair(pick, drop);
		vector<int> matable_taxis = filter_taxi();
		//match_time.resume();
		//cout << "have " << matable_taxis.size() << " taxis" << endl;
		for (int i = 0; i < matable_taxis.size(); ++i) {
			singleresult.clear();
			int taxiid = matable_taxis[i];
			cur_taxi = taxis[taxiid];
			request_insertion();
			if (!singleresult.empty()) {
				result.insert(result_t(singleresult.begin()->regret, singleresult.begin()->serve_regret,
					singleresult.begin()->revenue_regret,
					singleresult.begin()->deregret, singleresult.begin()->serve_dist,
					singleresult.begin()->unserve_dist,
					singleresult.begin()->serve_count, singleresult.begin()->unserve_count,
					singleresult.begin()->add_serve_dist, singleresult.begin()->detour,
					singleresult.begin()->taxiid, singleresult.begin()->schedule,
					singleresult.begin()->pick_pos, singleresult.begin()->drop_pos));
			}
		}
		//match_time.pause();
		if (result.size() != 0) {
			request_taxi_pair[r.id] = result;
			req_list[r.id] = r;
		}
		delete drop;
		delete pick;
	}
}

bool replace(int taxiid, request_t r, set<result_t> ress) {
	map<int, taxi_t> addreq = taxi_request[taxiid];
	for (auto iter = addreq.begin(); iter != addreq.end(); iter++) {
		int reqid = iter->first;
		request_t rq = req_list[reqid];
		set<result_t> rq_res = request_remain_result[reqid].first;
		set<result_t> rq_remain_res = request_remain_result[reqid].second;
		if (rq_remain_res.empty()) {
			continue;
		}
		double degr = rq_res.begin()->deregret;
		double det = rq_res.begin()->detour;

		cur_detour = cur_detour - rq_res.begin()->detour;
		cur_serve_dist = cur_serve_dist - rq_res.begin()->add_serve_dist;
		cur_serve_shortest_dist = cur_serve_shortest_dist - rq.dist_sd;
		cur_unserve_dist = cur_unserve_dist + rq.dist_sd;
		cur_unserve_count++;
		cur_serve_count--;

		singleresult.clear();
		cur_taxi = taxis[taxiid];
		vector<node_t*> schedule = cur_taxi.root;
		update_time.resume();
		for (int i = 1; i < schedule.size(); i++) {
			if (schedule[i]->id == reqid) {
				schedule.erase(schedule.begin() + i);
			}
			if (schedule[i]->id == reqid + 1) {
				schedule.erase(schedule.begin() + i);
				break;
			}
		}
		push(schedule);
		update_time.pause();
		//match_time.resume();
		request_insertion();
		//match_time.pause();
		if (singleresult.size() != 0) {
			double degret = singleresult.begin()->deregret;
			double detour = singleresult.begin()->detour;
			if (degr + eps < degret && det > detour + eps) {
				replacecount++;
				set<result_t> replace_req_res = request_remain_result[rq.id].second;
				request_taxi_pair[rq.id] = replace_req_res;
				request_remain_result.erase(rq.id);
				addreq.erase(rq.id);
				vector<node_t*> schedule = singleresult.begin()->schedule;
				update_time.resume();
				push(schedule);
				update_time.pause();
				request_remain_result[r.id] = make_pair(singleresult, ress);
				map<int, taxi_t> tmp = addreq;
				tmp[r.id] = taxis[taxiid];
				taxi_request[taxiid] = tmp;
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
				outputfile << r.id / 2 << " " << r.time << " " << r.pickup << " " << r.dropoff << " " << r.dist_sd
					<< " " << result.begin()->detour << " " << result.begin()->add_serve_dist
					<< " " << taxiid << endl;
				return true;
			}
		}

		cur_detour = cur_detour + rq_res.begin()->detour;
		cur_serve_dist = cur_serve_dist + rq_res.begin()->add_serve_dist;
		cur_serve_shortest_dist = cur_serve_shortest_dist + rq.dist_sd;
		cur_unserve_dist = cur_unserve_dist - rq.dist_sd;
		cur_unserve_count--;
		cur_serve_count++;
	}
	return false;
}

void replace_simulate(queue<request_t>& requests, vector<taxi_t>& taxis)
{
	int se = SERVICE_CONST * 10;
	int pi = PICKUP_CONST / 10;
	int al = alpha * 10;
	int bt = batch;

	if (flag) {
		outputfile.open(data_dir + "/res_syn/res10_2d_p_"
			+ to_string(MAX_PASSENGERS) + "_"
			+ to_string(MAX_TAXIS) + "_"
			+ to_string(se) + "_"
			+ to_string(pi) + "_"
			+ to_string(al) + "_"
			+ to_string(bt)
			+ ".txt");
	}
	else {
		outputfile.open(data_dir + "/res_syn/res10_p_"
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
		cout << "current time: " << logicalTime << endl;
		while (!requests.empty()) {
			if (requests.front().time > logicalTime + batch - 1) {
				break;
			}
			request_t r = requests.front();
			requests.pop();
			cur_reqs.push(r);
		}
		cur_request_count = cur_request_count + cur_reqs.size();
		cur_unserve_count = cur_unserve_count + cur_reqs.size();
		int cur_totaldist = 0;
		for (int i = 0; i < cur_reqs.size(); ++i) {
			request_t r = cur_reqs.front();
			cur_totaldist = cur_totaldist + r.dist_sd;
			cur_reqs.push(r);
			cur_reqs.pop();
		}
		cur_allreq_dist = cur_allreq_dist + cur_totaldist;
		cur_unserve_dist = cur_unserve_dist + cur_totaldist;
		request_taxi_pair.clear();
		req_list.clear();
		request_remain_result.clear();
		taxi_request.clear();
		match_time.resume();
		pair_request_taxi();
		while (!request_taxi_pair.empty()) {
			all_time.resume();
			request = req_list[request_taxi_pair.begin()->first];
			set<result_t> ress = request_taxi_pair.begin()->second;
			request_t r = request;
			node_t* pick = new node_t(r.id, r.pickup, r.count, true, inf);
			node_t* drop = new node_t(r.id + 1, r.dropoff, -r.count, false, SERVICE_CONST * r.dist_sd);
			request_node = make_pair(pick, drop);
			request_taxi_pair.erase(r.id);
			while (!ress.empty()) {
				int taxiid = ress.begin()->taxiid;
				cur_taxi = taxis[taxiid];
				double degret = ress.begin()->deregret;
				double detour = ress.begin()->detour;
				ress.erase(ress.begin());
				singleresult.clear();
				request_insertion();
				if (singleresult.size() != 0) {
					vector<node_t*> schedule = singleresult.begin()->schedule;
					update_time.resume();
					push(schedule);
					update_time.pause();
					request_remain_result[r.id] = make_pair(singleresult, ress);
					map<int, taxi_t> tmp = taxi_request[taxiid];
					tmp[r.id] = taxis[taxiid];
					taxi_request[taxiid] = tmp;
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
					outputfile << r.id / 2 << " " << r.time << " " << r.pickup << " " << r.dropoff << " " << r.dist_sd
						<< " " << result.begin()->detour << " " << result.begin()->add_serve_dist
						<< " " << taxiid << endl;
					break;
				}
				else {
					bool isreplace = replace(taxiid, r, ress);
					if (isreplace) {
						break;
					}
				}
			}
			delete drop;
			delete pick;
			all_time.pause();
		}
		match_time.pause();
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
	outputfile << "replace count: " << replacecount << endl;
	outputfile.close();
}