#define dump(x) cerr << #x << " = " << (x) << endl;
#define debug(x) cerr << #x << " = " << (x) << " (L" << __LINE__ << ")" \
					  << " " << __FILE__ << endl;
#include <cstdio>
#include <cassert>
#include <vector>
#include <tuple>
#include <iostream>
#include <queue>
using namespace std;


// from: https://atcoder.jp/contests/ahc002/submissions/22075413

const int64_t CYCLES_PER_SEC = 3000000000;
const double TIMELIMIT = 1.95;

struct Timer {
	int64_t start;
	Timer() { reset(); }
	void reset() { start = getCycle(); }
	void plus(double a) { start -= (a * CYCLES_PER_SEC); }
	inline double get() { return (double)(getCycle() - start) / CYCLES_PER_SEC; }
	inline int64_t getCycle() {
		uint32_t low, high;
		__asm__ volatile("rdtsc"
						 : "=a"(low), "=d"(high));
		return ((int64_t)low) | ((int64_t)high << 32);
	}
};

// from: https://atcoder.jp/contests/ahc002/submissions/22075413

class XorShift {
   public:
	unsigned int x, y, z, w;
	double nL[65536];
 
	XorShift() {
		init();
	}
 
	void init() {
		x = 314159265;
		y = 358979323;
		z = 846264338;
		w = 327950288;
		double n = 1 / (double)(2 * 65536);
		for (int i = 0; i < 65536; i++) {
			nL[i] = log(((double)i / 65536) + n);
		}
	}
 
	inline unsigned int next() {
		unsigned int t = x ^ x << 11;
		x = y;
		y = z;
		z = w;
		return w = w ^ w >> 19 ^ t ^ t >> 8;
	}
 
	inline double nextLog() {
		return nL[next() & 0xFFFF];
	}
 
	inline int nextInt(int m) {
		return (int)(next() % m);
	}
 
	int nextInt(int min, int max) {
		return min + nextInt(max - min + 1);
	}
 
	inline double nextDouble() {
		return (double)next() / ((long long)1 << 32);
	}
};
XorShift rnd;

const int HEIGHT = 50;
const int WIDTH = 50;

const string OP = "ULDR";
const int DW[4] = {0, -1, 0, 1};
const int DH[4] = {-1, 0, 1, 0};

struct Position {
  int h;
  int w;

  Position move(int i) {
    return {h+DH[i], w+DW[i]};
  }
};

struct Field {
  int t[HEIGHT][WIDTH];
  int p[HEIGHT][WIDTH];

  Field(Field const &field) {
    for(int h = 0; h < HEIGHT; ++h) {
      memcpy(t[h], field.t[h], sizeof(int)*WIDTH);
    }
    for(int h = 0; h < HEIGHT; ++h) {
      memcpy(p[h], field.p[h], sizeof(int)*WIDTH);
    }
  }

  Field() {};
};

enum class Action: int {
  Up = 0,
  Left = 1,
  Down = 2,
  Right = 3,
};

struct Reward {
  int depth;
};

struct Result {
  Reward reward;
  vector<int> actions;
};

struct State {
  Field field;
  Position cur_pos;
  bool used_t[HEIGHT*WIDTH+1] = {};
  int score = 0;
  int depth = 0;

  State(State const &state) {
    field = state.field;
    cur_pos = {state.cur_pos.h, state.cur_pos.w};
    memcpy(used_t, state.used_t, sizeof(bool)*(HEIGHT*WIDTH+1));
    score = state.score;
    depth = state.depth;
  }

  State(Field &field_, Position &cur_pos_) {
    field = field_;
    cur_pos = cur_pos_;
  }

  State() {};

  void show() {
    cout << string(10, '-') << endl;
    cout << "score: " << score << endl;
    cout << "depth: " << depth << endl;
    cout << "cur_pos: (" << cur_pos.h << ", " << cur_pos.w << ")" << endl;
    cout << "used_t: ";
    for(int t = 0; t < HEIGHT*WIDTH; ++t) {
      if (used_t[t]) {
        cout << t << ",";
      }
    }
    cout << endl;
  }

  bool can_apply(int action) {
    Position pos = cur_pos.move(action);
    if (pos.h < 0 || HEIGHT <= pos.h) {
      return false;
    }
    if (pos.w < 0 || WIDTH <= pos.w) {
      return false;
    }
    return !used_t[field.t[pos.h][pos.w]];
  }

  void apply(int action) {
    assert(can_apply(action));
    cur_pos = cur_pos.move(action);
    used_t[field.t[cur_pos.h][cur_pos.w]] = true;
    score += field.p[cur_pos.h][cur_pos.w];
    depth += 1;
  }
};

struct Node {
  Node* parent = nullptr;
  State state;
  int pre_action = -1;
  vector<Node> children;
  bool expanded;

  Node(State& state_, int pre_action_) {
    state = state_;
    pre_action = pre_action_;
  }

  Node(State& state_) {
    state = state_;
  }

  Node() {}

  Node get_child_node() {
    return get_child_node_randomly();
  }

  Node get_child_node_randomly() {
    return children[rnd.nextInt(children.size())];
  }

  Node get_child_node_with_max_score() {
    return children[0];
  }

  bool has_child() {
    return !children.empty();
  }

  void expand() {
    for(int i = 0; i < 4; ++i) {
      if(!state.can_apply(i)) {
        continue;
      }
      State s = State(state);
      s.apply(i);
      Node n = Node(s, i);
      children.push_back(n);
    }
  }

};

struct Tree {
  Node root;

  Tree(Node& root_) {
    root = root_;
  }

  Tree() {}

  //traverseとかしたくなったらここへブチ込む
};

struct MonteCarloTreeSearch {
  Tree tree;
  Timer timer;
  Result best_result;

  MonteCarloTreeSearch(Tree& tree_, Timer& timer_) {
    tree = tree_;
    timer = timer_;
  }

  vector<int> search() {
    while(timer.get() <= TIMELIMIT) {
      // selection
      Node promising_node = select_promising_node();
    
      // expansion
      if(!promising_node.expanded) {
        promising_node.expand();
      }

      // simulation
      Node node_to_explore = promising_node;
      if (promising_node.has_child()) {
        node_to_explore = promising_node.get_child_node_randomly();
      }
      Result result = simulateRandomPlayout(node_to_explore);

      // update
      back_propagation(node_to_explore, result);
    }
    return best_result.actions;
  }

  vector<int> get_best_actions() {
    return vector<int>();
  }

  Node select_promising_node() {
    Node node = tree.root;
    while(node.expanded) {
      node = node.get_child_node_randomly();
    }
    return node;
  }

  void back_propagation(Node& node, Result& result) {
    if (best_result.reward.depth < result.reward.depth) {
      best_result = result;
      //cout << "updated" << endl;
      //for(int i = 0; i < best_result.actions.size(); ++i) {
      //  cout << OP[best_result.actions[i]];
      //}
      //cout << endl;
    }
    while(node.parent != nullptr) {
      node = *node.parent;
      // do something
    }
    return;
  }

  vector<int> get_cur_actions(Node& node) {
    vector<int> actions;
    actions.push_back(node.pre_action);
    while(node.parent != nullptr) {
      node = *node.parent;
      actions.push_back(node.pre_action);
    }
    reverse(actions.begin(), actions.end());
    return actions;
  }

  Result simulateRandomPlayout(Node& node) {    
    State state = State(node.state);
    vector<int> actions = get_cur_actions(node);
    while(1) {
      int action = rnd.nextInt(4);
      if (!state.can_apply(action)) {
        break;
      }
      actions.push_back(action);
      state.apply(action);
      //state.show();
    }
    return {{state.depth}, actions};
  }
};

struct Solver {
  Field field;
  Position start_pos;
  Timer timer;
  vector<int> actions;

  void solve() {
    State state = State(field, start_pos);
    state.used_t[field.t[start_pos.h][start_pos.w]] = true;
    Node node = Node(state);
    Tree tree = Tree(node);
    auto mcts = MonteCarloTreeSearch(tree, timer);
    actions = mcts.search();
    return;

    node.expand();
    while(node.has_child()) {
      node = node.get_child_node_randomly();
      node.expand();
      actions.push_back(node.pre_action);
    } 
    actions.push_back(node.pre_action);
  }

  void input() {
    scanf("%d %d", &start_pos.h, &start_pos.w);
    for (int i = 0; i < HEIGHT; i++) {
      for (int j = 0; j < WIDTH; j++) {
        scanf("%d", &field.t[i][j]);
      }
    }
    for (int i = 0; i < HEIGHT; i++) {
      for (int j = 0; j < WIDTH; j++) {
        scanf("%d", &field.p[i][j]);
      }
    }
  }

  void output() {
    string res;
    for (int i = 0; i < actions.size(); i++) {
      res.push_back(OP[actions[i]]);
    }
    cout << res << endl;
  }
};

int main() {
  ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  Solver solver;
  solver.input();
  solver.solve();
  solver.output();
  debug(solver.timer.get());
  return 0;
}
