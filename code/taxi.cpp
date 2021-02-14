#include "main.h"

taxi_t cur_taxi;
pair<node_t*, node_t*> request_node;

node_t::node_t(int vert, int passenger, int count) //for root
	:id(-1), vert(vert), count(count), passenger(passenger), parent(NULL),
	dist_parent(0), dist_root(0)
{
}
//onborad: alrady pickup
node_t::node_t(long id, int vert, int count, bool start, double limit) //for request
	: id(id), vert(vert), count(count), limit(limit), start(start), onboard(false)
{
}

node_t::node_t(node_t* t, parent_t* parent, double dist_parent) //for building the tree
	: id(t->id), vert(t->vert), count(t->count), parent(parent), limit(t->limit),
	dist_parent(dist_parent), dist_root(parent->dist_root + dist_parent),
	start(t->start), onboard(t->onboard)
{
}

node_t::~node_t() {

}

bool check_limit(int position)
{
	node_t* node = cur_taxi.root[position];
	return request.detour <= node->dist_slack;
}

void insert_s(int position)
{
	if (position == cur_taxi.root.size()) {
		return;
	}
	node_t* node = cur_taxi.root[position];
	request_t& r = request;
	bool ok(true);
	bool pick(true);
	int dcheck = prune_s(position);
	if (flag) {
		if (cur_taxi.root[position - 1]->passenger + r.count <= MAX_PASSENGERS && dcheck == 1) {
			//cp[i]+rn<=c
			double dist_xs = get_dist(cur_taxi.root[position - 1]->vert, r.pickup);
			double dist_sy = get_dist(r.pickup, node->vert);
			double detour = dist_xs + dist_sy - (node->dist_root - cur_taxi.root[position - 1]->dist_root);
			r.detour = detour; //det(o_i,l_s,o_{i+1})=dis(o_i,l_s)+dis(l_s,o_{i+1})-dis(o_i,o_{i+1})
			r.pickup_dist = cur_taxi.root[position - 1]->dist_root + dist_xs + cur_taxi.dist;//arr[i]+dis(o_i,l_s)
			r.service_time = dist_sy - node->dist_root;
			node->dist_parent += detour;
			pick = (r.pickup_dist <= PICKUP_CONST); //arr[i]+dis(o_i,l_s)<=theta_p
			ok = check_limit(position); //det(o_i,l_s,o_{i+1})<=slk[i+1]
			if (pick && ok) {
				if (position == cur_taxi.root.size()) {
					insert_d_end(position, position - 1);
				}
				else {
					insert_d(position, position + 1);
				}
			}
			node->dist_parent -= detour;
			if (pick) {
				insert_s(position + 1);
			}
		}
		if (dcheck != -1) {
			insert_s(position + 1);
		}
	}
	else {
		int passenger = 0;
		for (int i = 0; i < position; i++) {
			passenger = passenger + cur_taxi.root[i]->count;
		}
		if (passenger + r.count <= MAX_PASSENGERS) {
			//cp[i]+rn<=c
			double dist_xs = get_dist(cur_taxi.root[position - 1]->vert, r.pickup);
			double dist_sy = get_dist(r.pickup, node->vert);
			double dist_root = 0;
			for (int i = 1; i < position; i++) {
				dist_root = dist_root + cur_taxi.root[i]->dist_parent;
			}
			double dist_root1 = dist_root + node->dist_parent;
			double detour = dist_xs + dist_sy - (node->dist_root - cur_taxi.root[position - 1]->dist_root);
			r.detour = detour; //det(o_i,l_s,o_{i+1})=dis(o_i,l_s)+dis(l_s,o_{i+1})-dis(o_i,o_{i+1})
			r.pickup_dist = cur_taxi.root[position - 1]->dist_root + dist_xs + cur_taxi.dist;//arr[i]+dis(o_i,l_s)
			r.service_time = dist_sy - node->dist_root;
			node->dist_parent += detour;
			pick = (r.pickup_dist <= PICKUP_CONST); //arr[i]+dis(o_i,l_s)<=theta_p
			double limit = inf;
			for (int i = position; i < cur_taxi.root.size(); i++) {
				double lim = cur_taxi.root[i]->limit - cur_taxi.root[i]->dist_root;
				if (lim < limit) {
					limit = lim;
				}
			}
			if (detour > limit) {
				ok = false;
			}
			if (pick && ok) {
				if (position == cur_taxi.root.size()) {
					insert_d_end(position, position - 1);
				}
				else {
					insert_d(position, position + 1);
				}
			}
			node->dist_parent -= detour;
			if (pick) {
				insert_s(position + 1);
			}
		}
		insert_s(position + 1);

	}
}

void insert_d(int pick_pos, int position)
{
	node_t* node = cur_taxi.root[position];
	request_t& r = request;
	bool ok(true);
	bool drop(true);
	if (flag) {
		if (cur_taxi.root[position - 1]->passenger + r.count > MAX_PASSENGERS) {//cp[j]+rn<=c
			return;
		}
	}
	else {
		double passenger = 0;
		for (int i = 0; i < position; i++) {
			passenger = passenger + cur_taxi.root[i]->count;
		}
		if (passenger + r.count > MAX_PASSENGERS) {//cp[j]+rn<=c
			return;
		}
	}
	if (position == cur_taxi.root.size()) {
		insert_d_end(pick_pos, position - 1);
	}
	else {
		if (flag) {
			if (!prune_d(position)) {
				double dist_md = get_dist(cur_taxi.root[position - 1]->vert, r.dropoff);
				double dist_dn = get_dist(r.dropoff, node->vert);
				double service = dist_md + cur_taxi.root[position - 1]->dist_root + r.service_time;
				//dis(l_s, o_{i+1})-arr[i+1]+dis(o_j, l_d)+arr[j]=dis(l_s,o_{i+1})+dis(o_{i+1},o_j)+dis(o_j, l_d)
				double before = r.service_time;
				double detour = dist_md + dist_dn - node->dist_parent;
				//det(o_j,l_d,o_{j+1})=dis(o_j,l_d)+dis(l_d,o_{j+1})-dis(o_j,o_{j+1})
				double add_served_dist = r.detour * cur_taxi.root[pick_pos - 1]->passenger;
				//cp[i]*det(o_i,l_s,o_{i+1})
				r.detour += detour; //det(o_i,l_s,o_{i+1})+det(o_j,l_d,o_{j+1})
				add_served_dist = add_served_dist + cur_taxi.root[position - 1]->passenger * detour + service;
				//cp[i]*det(o_i,l_s,o_{i+1})+cp[j]*det(o_j,l_d,o_{j+1})+dis(l_s,o_{i+1})+dis(o_{i+1},o_j)+dis(o_j,l_d)
				r.service_time = service;
				//insert_check(pick_pos, position, add_served_dist);
				ok = check_limit(position);
				//\Delta_{i,j}<=slk[j+1]
				drop = (r.service_time + r.pickup_dist <= SERVICE_CONST * r.dist_sd);
				//arr[j]+det(o_i,l_s,o_{i+1})+dis(o_j,l_d)<=theta_d
				if (ok && drop) {
					insert_result(pick_pos, position + 1, add_served_dist);
				}
				r.service_time = before;
				r.detour -= detour;
				if (drop) {
					insert_d(pick_pos, position + 1);
				}
			}
		}
		else {
			double dist_md = get_dist(cur_taxi.root[position - 1]->vert, r.dropoff);
			double dist_dn = get_dist(r.dropoff, node->vert);
			double dist_root = 0;
			for (int i = 1; i < position; i++) {
				dist_root = dist_root + cur_taxi.root[i]->dist_parent;
			}
			double service = dist_md + cur_taxi.root[position - 1]->dist_root + r.service_time;
			//dis(l_s, o_{i+1})-arr[i+1]+dis(o_j, l_d)+arr[j]=dis(l_s,o_{i+1})+dis(o_{i+1},o_j)+dis(o_j, l_d)
			double before = r.service_time;
			double detour = dist_md + dist_dn - node->dist_parent;
			//det(o_j,l_d,o_{j+1})=dis(o_j,l_d)+dis(l_d,o_{j+1})-dis(o_j,o_{j+1})
			double add_served_dist = r.detour * cur_taxi.root[pick_pos - 1]->passenger;
			//cp[i]*det(o_i,l_s,o_{i+1})
			r.detour += detour; //det(o_i,l_s,o_{i+1})+det(o_j,l_d,o_{j+1})
			add_served_dist = add_served_dist + cur_taxi.root[position - 1]->passenger * detour + service;
			//cp[i]*det(o_i,l_s,o_{i+1})+cp[j]*det(o_j,l_d,o_{j+1})+dis(l_s,o_{i+1})+dis(o_{i+1},o_j)+dis(o_j,l_d)
			r.service_time = service;
			//insert_check(pick_pos, position, add_served_dist);
			double limit = inf;
			for (int i = position; i < cur_taxi.root.size(); i++) {
				double lim = cur_taxi.root[i]->limit - cur_taxi.root[i]->dist_root;
				if (lim < limit) {
					limit = lim;
				}
			}
			if (detour > limit) {
				ok = false;
			}

			//\Delta_{i,j}<=slk[j+1]
			drop = (r.service_time + r.pickup_dist <= SERVICE_CONST * r.dist_sd);
			//arr[j]+det(o_i,l_s,o_{i+1})+dis(o_j,l_d)<=theta_d
			if (ok && drop) {
				insert_result(pick_pos, position + 1, add_served_dist);
			}
			r.service_time = before;
			r.detour -= detour;
			if (drop) {
				insert_d(pick_pos, position + 1);
			}
		}
	}
}

void insert_d_end(int pick_pos, int position)
{
	node_t* node = cur_taxi.root[position];
	double dist_md = get_dist(node->vert, request.dropoff);
	double service = 0;
	if (!flag) {
		double dist_root = 0;
		for (int i = 1; i < position + 1; i++) {
			dist_root = dist_root + cur_taxi.root[i]->dist_parent;
		}
	}
	service = dist_md + node->dist_root + request.service_time;
	//dis(l_s, o_{i+1})-dis(o_0, o_{i+1})+dis(o_n, l_d)+arr[n]=dis(l_s,o_{i+1})+dis(o_{i+1},o_n)+dis(o_n, l_d)
	double before = request.service_time;
	double detour = dist_md; //dis(o_n, l_d)
	if (service + request.pickup_dist <= SERVICE_CONST * request.dist_sd) {
		double add_served_dist = service + request.detour * cur_taxi.root[pick_pos - 1]->passenger;
		//cp[i]*det(o_i,l_s,o_{i+1})+dis(l_s,o_{i+1})+dis(o_{i+1},o_n)+dis(o_n, l_d)
		request.detour += detour;//det(o_i,l_s,o_{i+1})+dis(o_n, l_d)
		request.service_time = service;
		insert_result(position, position + 2, add_served_dist);
		request.service_time = before;
		request.detour -= detour;
	}
}

void insert_sd(int position)
{
	if (position == cur_taxi.root.size()) {
		insert_sd_end(position - 1);
	}
	else {
		node_t* node = cur_taxi.root[position];
		request_t& r = request;
		int dcheck = prune_sd(position);
		if (flag) {
			if (dcheck == 1 && cur_taxi.root[position - 1]->passenger + r.count <= MAX_PASSENGERS) {
				double dist_xs = get_dist(cur_taxi.root[position - 1]->vert, r.pickup);
				double dist_dn = get_dist(r.dropoff, node->vert);
				double detour = dist_xs + dist_dn + r.dist_sd
					- (node->dist_root - cur_taxi.root[position - 1]->dist_root);
				//dis(o_i,l_s)+dis(l_s,l_d)+dis(l_d.o_{i+1})-dis(o_i,o_{i+1})
				r.detour = detour;
				r.pickup_dist = cur_taxi.root[position - 1]->dist_root + dist_xs + cur_taxi.dist;
				//arr[i]+dis(o_i,l_s)
				r.service_time = r.dist_sd;
				if (r.pickup_dist <= PICKUP_CONST && r.pickup_dist + r.dist_sd <= SERVICE_CONST * r.dist_sd) {
					//arr[i]+dis(o_i,l_s)<=theta_p && arr[i]+dis(o_i,l_s)+dis(l_s,l_d)<=thet_d	
					bool ok = check_limit(position);//Delta_{i,j}<=slk[j+1]
					double add_served_dist = detour * cur_taxi.root[position - 1]->passenger + r.dist_sd;
					//cp[i]*\Delta_{i,j}+dis(l_s,l_d)
					if (ok) {
						insert_result(position, position + 1, add_served_dist);
					}
					insert_sd(position + 1);
				}
			}
			if (dcheck != -1) {
				insert_sd(position + 1);
			}
		}
		else {
			double passenger = 0;
			for (int i = 0; i < position; i++) {
				passenger = passenger + cur_taxi.root[i]->count;
			}
			if (passenger + r.count <= MAX_PASSENGERS) {
				double dist_xs = get_dist(cur_taxi.root[position - 1]->vert, r.pickup);
				double dist_dn = get_dist(r.dropoff, node->vert);
				double dist_root = 0;
				for (int i = 1; i < position; i++) {
					dist_root = dist_root + cur_taxi.root[i]->dist_parent;
				}
				double dist_root1 = dist_root + node->dist_parent;
				double detour = dist_xs + dist_dn + r.dist_sd
					- (node->dist_root - cur_taxi.root[position - 1]->dist_root);
				r.detour = detour;
				r.pickup_dist = cur_taxi.root[position - 1]->dist_root + dist_xs + cur_taxi.dist;
				//arr[i]+dis(o_i,l_s)
				r.service_time = r.dist_sd;
				if (r.pickup_dist <= PICKUP_CONST && r.pickup_dist + r.dist_sd <= SERVICE_CONST * r.dist_sd) {
					//arr[i]+dis(o_i,l_s)<=theta_p && arr[i]+dis(o_i,l_s)+dis(l_s,l_d)<=thet_d	
					double add_served_dist = detour * cur_taxi.root[position - 1]->passenger + r.dist_sd;
					bool ok(true);
					double limit = inf;
					for (int i = position; i < cur_taxi.root.size(); i++) {
						double lim = cur_taxi.root[i]->limit - cur_taxi.root[i]->dist_root;
						if (lim < limit) {
							limit = lim;
						}
					}
					if (detour > limit) {
						ok = false;
					}
					//cp[i]*\Delta_{i,j}+dis(l_s,l_d)
					if (ok) {
						insert_result(position, position + 1, add_served_dist);
					}
					insert_sd(position + 1);
				}
			}
			insert_sd(position + 1);
		}
	}
}

void insert_sd_end(int position)
{
	node_t* node = cur_taxi.root[position];
	double dist_xs = get_dist(node->vert, request.pickup);
	if (flag) {
		request.pickup_dist = node->dist_root + dist_xs + cur_taxi.dist;
	}
	else {
		double dist_root = 0;
		for (int i = 1; i <= position; i++) {
			dist_root = dist_root + cur_taxi.root[i]->dist_parent;
		}
		request.pickup_dist = node->dist_root + dist_xs + cur_taxi.dist;
	}
	request.detour = dist_xs + request.dist_sd; //dis(o_n,l_s)+dis(l_s,l_d)
	request.service_time = request.dist_sd;
	if (request.pickup_dist <= PICKUP_CONST
		&& request.pickup_dist + request.dist_sd <= SERVICE_CONST * request.dist_sd) {
		//arr[i]+dis(o_i,l_s)<=theta_p && arr[i]+dis(o_i,l_s)+dis(l_s,l_d)<=thet_d
		insert_result(position + 1, position + 2, request.service_time);
	}
}

int prune_s(int position)
{
	double dist_xs = ldist(cur_taxi.root[position - 1]->vert, request.pickup);
	double dist_sy = ldist(request.pickup, cur_taxi.root[position]->vert);
	double pickup_dist = cur_taxi.root[position - 1]->dist_root + dist_xs + cur_taxi.dist;
	double detour = dist_xs + dist_sy - (cur_taxi.root[position]->dist_root - cur_taxi.root[position - 1]->dist_root);
	if (pickup_dist > PICKUP_CONST) {//arr[i]+ldis(o_i,l_s)<=theta_p
		return -1;
	}
	if (detour > cur_taxi.root[position]->dist_slack) {
		//ldis(o_i,l_s)+ldis(l_s,o_{i+1})-dis(o_i,o_{i+1})<=slk[i]
		return 0;
	}
	return 1;
}

bool prune_d(int position)
{
	request_t& r = request;
	double dist_md = ldist(cur_taxi.root[position - 1]->vert, r.dropoff);
	double dist_dn = ldist(r.dropoff, cur_taxi.root[position]->vert);
	double service = dist_md + cur_taxi.root[position - 1]->dist_root + r.service_time;
	double detour = dist_md + dist_dn - cur_taxi.root[position]->dist_parent;

	return detour > cur_taxi.root[position]->dist_slack || (service + r.pickup_dist) > SERVICE_CONST * r.dist_sd;
}

int prune_sd(int position)
{
	request_t& r = request;
	double dist_xs = ldist(cur_taxi.root[position - 1]->vert, r.pickup);
	double dist_dn = ldist(r.dropoff, cur_taxi.root[position]->vert);
	double pickup_dist = cur_taxi.root[position - 1]->dist_root + dist_xs + cur_taxi.dist;
	double detour = dist_xs + dist_dn + r.dist_sd - (cur_taxi.root[position]->dist_root - cur_taxi.root[position - 1]->dist_root);
	if (pickup_dist > PICKUP_CONST //arr[i]+ldis(o_i,l_s)
		|| (pickup_dist + r.dist_sd) > SERVICE_CONST * r.dist_sd) {
		//arr[i]+ldis(o_i,l_s)+dis(l_s,l_d)
		return -1;
	}
	if (detour > cur_taxi.root[position]->dist_slack) {
		//ldis(o_i,l_s)+dis(l_s,l_d)+ldis(l_d,o_{i+1})-dis(o_i,o_{i+1})
		return 0;
	}
	return 1;
}

void calc_slack(int position)
{
	double slack = inf;
	if (position == cur_taxi.root.size() - 1) {
		cur_taxi.root[position]->dist_slack = cur_taxi.root[position]->limit - cur_taxi.root[position]->dist_root;;
		return;
	}
	if (cur_taxi.root[position]->id != -1) {
		slack = cur_taxi.root[position]->limit - cur_taxi.root[position]->dist_root;
	}
	calc_slack(position + 1);
	cur_taxi.root[position]->dist_slack = min(cur_taxi.root[position + 1]->dist_slack, slack);
}

void evaluate() {
	vector<node_t*> root = cur_taxi.root;
	if (root.size() == 1) { //no chlidren
		insert_sd_end(0);
	}
	else {
		insert_sd(1);
		insert_s(1);
	}
}

void pickup(long id, int position)
{
	if (id == cur_taxi.root[position]->id - 1) {
		cur_taxi.root[position]->onboard = true;
		return;
	}
	for (int i = position + 1; i < cur_taxi.root.size(); i++) {
		pickup(id, i);
	}
}

void arrive()
{
	if (cur_taxi.root.size() == 1) {
		return;
	}
	int firstid = cur_taxi.root[0]->id;
	vector<node_t*>::iterator firstvert = cur_taxi.root.begin();
	cur_taxi.root.erase(firstvert);
	if (cur_taxi.root[0]->id % 2 == 0) {
		pickup(cur_taxi.root[0]->id, 0);
	}
	cur_taxi.root[0]->id = firstid;
	cur_taxi.root[0]->parent = NULL;
}

double get_next_vert()
{
	//update_dfs(false, DELETE, cur_taxi.root);
	int pre = cur_taxi.cur_path.back();
	cur_taxi.cur_path.pop_back();

	if (cur_taxi.cur_path.empty()) {
		//update_time.resume();
		arrive();
		//update_time.pause();
		if (cur_taxi.root.size() == 1) {
			cur_taxi.stop = true;
			return 0;
		}
		cur_taxi.cur_path = get_path(cur_taxi.root[1]->vert, cur_taxi.root[0]->vert);
	}
	cur_taxi.root[0]->vert = cur_taxi.cur_path.back();
	if (cur_taxi.root.size() > 1) {
		cur_taxi.root[1]->parent->vert = cur_taxi.root[0]->vert;
	}
	cur_taxi.gid = Node[cur_taxi.root[0]->vert].grid;
	return get_dist(pre, cur_taxi.cur_path.back());
}

void move()
{
	if (cur_taxi.stop) {
		return;
	}
	cur_taxi.dist -= speed*batch;
	if (cur_taxi.dist >= 0) {
		return;
	}
	while (cur_taxi.dist < 0) {
		double _dist = get_next_vert();
		if (cur_taxi.stop) {
			cur_taxi.dist = 0;
			break;
		}
		taxi_dist = taxi_dist + _dist;
		cur_taxi.dist += _dist;
		double dist_diff = cur_taxi.root[1]->dist_root - get_dist(cur_taxi.root[0]->vert, cur_taxi.root[1]->vert);
		for (int i = 1; i < cur_taxi.root.size(); i++) {
			cur_taxi.root[i]->dist_root -= dist_diff;
			if (cur_taxi.root[i]->start || cur_taxi.root[i]->onboard) {
				cur_taxi.root[i]->limit -= dist_diff;
			}
			parent_t* parent = new parent_t(cur_taxi.root[i - 1]->id, cur_taxi.root[i - 1]->vert, cur_taxi.root[i - 1]->count,
				cur_taxi.root[i - 1]->passenger, cur_taxi.root[i - 1]->limit, cur_taxi.root[i - 1]->dist_parent,
				cur_taxi.root[i - 1]->dist_root, cur_taxi.root[i - 1]->start, cur_taxi.root[i - 1]->onboard);
			cur_taxi.root[i]->parent = parent;
		}
	}
}

node_t* get_new_node(int position)
{
	node_t* node = cur_taxi.root[position];
	node_t* parent = cur_taxi.root[position - 1];
	double dist_parent;
	dist_parent = get_dist(parent->vert, node->vert);
	double dist_cnt = dist_parent;
	double dist_pair = 0;
	dist_cnt += parent->dist_root;
	parent_t* realparent = new parent_t(parent->id, parent->vert, parent->count, parent->passenger,
		parent->limit, parent->dist_parent, parent->dist_root, parent->start, parent->onboard);
	return dist_cnt > node->limit ? NULL : new node_t(node, realparent, dist_parent);
}

bool insert()
{
	int passenger = cur_taxi.root[0]->passenger;
	for (int i = 1; i < cur_taxi.root.size(); i++) {
		node_t* node = cur_taxi.root[i];
		parent_t* parent = new parent_t(cur_taxi.root[i - 1]->id, cur_taxi.root[i - 1]->vert, cur_taxi.root[i - 1]->count,
			cur_taxi.root[i - 1]->passenger, cur_taxi.root[i - 1]->limit, cur_taxi.root[i - 1]->dist_parent,
			cur_taxi.root[i - 1]->dist_root, cur_taxi.root[i - 1]->start, cur_taxi.root[i - 1]->onboard);
		cur_taxi.root[i]->parent = parent;
		passenger = passenger + node->count;
		node->passenger = passenger;
		if (passenger > MAX_PASSENGERS) {
			return false;
		}
		node_t* new_node = get_new_node(i);
		if (new_node != NULL) {
			new_node->passenger = node->passenger;
			cur_taxi.root[i] = new_node;
		}
		else {
			return false;
		}
	}
	for (int i = 1; i < cur_taxi.root.size(); i++) {
		//we're at the pickup node of the new request
		if (cur_taxi.root[i]->id == request.id) {
			cur_taxi.root[i]->limit = PICKUP_CONST;
			break;
		}
	}
	return true;
}

void push(vector<node_t*> schedule)
{
	vector<node_t*> temp = cur_taxi.root_temp;
	cur_taxi.root_temp = cur_taxi.root;
	cur_taxi.root = schedule;
	if (!insert()) {
		cur_taxi.root = cur_taxi.root_temp;
		cur_taxi.root_temp = temp;
	}
	calc_slack(0);
	cur_taxi.cur_path = get_path(cur_taxi.root[1]->vert, cur_taxi.root[0]->vert);
	cur_taxi.stop = false;
}

taxi_t init(int vertex)
{
	taxi_t t;
	t.dist = 0;
	vector<node_t*> root;
	node_t* rootnode = new node_t(vertex, 0, 0);
	root.push_back(rootnode);
	t.root = root;
	t.root_temp = root;
	t.stop = true;
	t.served_dist = 0;
	t.gid = Node[vertex].grid;

	return t;
}

vector<taxi_t> set_taxi(int n)
{
	vector<taxi_t> taxis;
	ifstream taxi_file((data_dir + "/taxi50k.dat").c_str());
	int i = 0;
	int vertex;
	while (taxi_file >> vertex) {
		taxi_t t = init(vertex);
		t.id = i;
		taxis.push_back(t);
		i++;
		if (i == n) {
			break;
		}
	}
	return taxis;
}

vector<int> filter_taxi()
{
	vector<int> matable_taxis;
	map<grid_id, bool> inavailable;
	for (int i = 0; i < MAX_TAXIS; ++i) {
		int pick_pos = request.pickup;
		int taxi_pos = taxis[i].root[0]->vert;
		if (inavailable.count(taxis[i].gid) != 0) {
			if (flag) {
				continue;
			}
		}
		double lgpick = ldist(pick_pos, taxis[i].gid);
		if (lgpick > PICKUP_CONST) {
			inavailable[taxis[i].gid] = true;
			if (flag) {
				continue;
			}
		}
		double lpick = ldist(pick_pos, taxi_pos);
		if (lpick > PICKUP_CONST) {
			if (flag) {
				continue;
			}
		}
		//double lpick = 1000*hsdist(vertices[pick_pos]->y, vertices[pick_pos]->x, vertices[taxi_pos]->y, vertices[taxi_pos]->x);
		//cout << "lower distance: " << lpick << endl;
		//if (lpick > PICKUP_CONST) {
			//continue;
		//}
		double pick = get_dist(pick_pos, taxi_pos);
		if (pick > PICKUP_CONST) {
			if (flag) {
				continue;
			}
		}
		matable_taxis.push_back(taxis[i].id);
	}
	return matable_taxis;
}