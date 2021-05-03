#pragma GCC target("avx2")
#pragma GCC optimize("O3")
#pragma GCC optimize("unroll-loops")
#define dump(x) cerr << #x << " = " << (x) << endl;
#define debug(x) cerr << #x << " = " << (x) << " (L" << __LINE__ << ")" \
					  << " " << __FILE__ << endl;
#include <cstdio>
#include <vector>
#include <tuple>
#include <iostream>
#include <queue>
using namespace std;

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

const int HEIGHT = 50;
const int WIDTH = 50;

const string OP = "ULDR";
const int DW[4] = {0, -1, 0, 1};
const int DH[4] = {-1, 0, 1, 0};

struct Position {
  int h;
  int w;
};

struct Field {
  int t[HEIGHT][WIDTH];
  int p[HEIGHT][WIDTH];
};

struct Solver {
  Field field;
  Position start_pos;
  Timer timer;
  bool used_t[HEIGHT*WIDTH+1];
  vector<int> ops;
  vector<int> best_ops;
  long long steps;
  vector<int> depth_steps;

  Solver(Field field_, Position start_pos_) {
    field = field_;
    start_pos = start_pos_;
  }

  bool can_move(Position pos) {
    if (pos.h < 0 || HEIGHT <= pos.h) {
      return false;
    }
    if (pos.w < 0 || WIDTH <= pos.w) {
      return false;
    }
    return !used_t[field.t[pos.h][pos.w]];
  }

  void dfs(Position cur_pos, int depth) {
    steps += 1;
    depth_steps[depth] += 1;
    if (timer.get() >= TIMELIMIT) {
      return;
    }
    bool updated = false;
    for(int d = 0; d < 4; ++d) {
      Position n_pos = {cur_pos.h+DH[d], cur_pos.w+DW[d]};
      if (!can_move(n_pos)) {
        continue;
      }
      updated = true;
      used_t[field.t[n_pos.h][n_pos.w]] = true;            
      ops.push_back(d);
      dfs(n_pos, depth+1);
      ops.pop_back();
      used_t[field.t[n_pos.h][n_pos.w]] = false;                        
    }
    if (!updated && ops.size() > best_ops.size()) {
      best_ops = vector<int>(ops);
    }
  }
  
  void solve() {
    steps = 0;
    depth_steps = vector<int>(HEIGHT*WIDTH+1);    
    used_t[field.t[start_pos.h][start_pos.w]] = true;
    dfs(start_pos, 0);
  }

  void output() {
    string res;
    for (int i = 0; i < best_ops.size() - 1; i++) {
      res.push_back(OP[best_ops[i]]);
    }
    cout << res << endl;
  }
};

int main() {
  ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  Field f;
  Position pos;
  scanf("%d %d", &pos.h, &pos.w);
  for (int i = 0; i < HEIGHT; i++) {
    for (int j = 0; j < WIDTH; j++) {
      scanf("%d", &f.t[i][j]);
    }
  }
  for (int i = 0; i < HEIGHT; i++) {
    for (int j = 0; j < WIDTH; j++) {
      scanf("%d", &f.p[i][j]);
    }
  }
  Solver solver(f, pos);
  solver.solve();
  solver.output();
  cerr << solver.timer.get() << endl;
  cerr << solver.steps << endl;
  for(int i = 0; i < solver.depth_steps.size(); ++i) {
    if(solver.depth_steps[i] == 0) {
      break;
    }
    cerr << i << ": " << solver.depth_steps[i] << endl;
  }
  return 0;
}
