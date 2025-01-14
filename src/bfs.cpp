#include <cstdint>
#include <graph.hpp>
#include <iostream>
#include <omp.h>
#include <vector>
#include <profiling.hpp>

namespace bfs {

constexpr int ALPHA = 14;
constexpr int BETA = 24;
constexpr int MARKED_BIT = 31;
constexpr int MARKED_MASK = 1 << MARKED_BIT;
constexpr int VISITED_BIT = 30;
constexpr int VISITED_MASK = 0b01 << VISITED_BIT;
constexpr int MARKED_VISITED_MASK = 0b11 << VISITED_BIT;

enum class Direction { TOP_DOWN, BOTTOM_UP };
typedef std::vector<vidType> frontier;

#define IS_VISITED(i) (((merged[i]) & VISITED_MASK) != 0)
#define IS_MARKED(i) (((merged[i]) & MARKED_MASK) != 0)

class Graph : public BaseGraph {
  eidType *rowptr;
  [[maybe_unused]] vidType *col;
  vidType *merged;
  uint32_t unexplored_edges;
  Direction dir;
  [[maybe_unused]] uint64_t N;
  [[maybe_unused]] uint64_t M;
  uint32_t edges_frontier, edges_frontier_old;

public:
  Graph(eidType *rowptr, vidType *col, vidType *merged, uint64_t N, uint64_t M)
      : rowptr(rowptr), col(col), merged(merged), N(N), M(M) {
    unexplored_edges = M;
  }
  ~Graph() {}

  inline vidType copy_unmarked(vidType i) { return merged[i] & ~(MARKED_VISITED_MASK); }

  inline void set_distance(vidType i, weight_type distance) {
    merged[i] = distance | MARKED_VISITED_MASK;
  }

  inline bool is_unconnected(vidType i) { return (merged[i] & ~(MARKED_VISITED_MASK)) == 0; }

  inline bool is_leaf(vidType i) { return (rowptr[i] == rowptr[i + 1] - 1); }

  void compute_distances(weight_type *distances, vidType source) {
    #pragma omp parallel for schedule(auto)
    for (uint64_t i = 0; i < N; i++) {
      if (IS_VISITED(rowptr[i])) {
        distances[i] = copy_unmarked(rowptr[i]);
      }
    }
    distances[source] = 0;
  }

  void print_frontier(frontier &frontier) {
    std::cout << "Frontier: ";
    for (const auto &v : frontier) {
      std::cout << v << " ";
    }
    std::cout << std::endl;
  }

  inline void add_to_frontier(frontier &frontier, vidType v) {
    frontier.push_back(v);
    edges_frontier += copy_unmarked(v);
  }

  #pragma omp declare reduction(vec_add: std::vector<vidType>, std::vector<std::pair<vidType, bool>>: \
    omp_out.insert(omp_out.end(), omp_in.begin(), omp_in.end()))

  void bottom_up_step(frontier this_frontier, frontier &next_frontier, weight_type distance) {
    // std::cout << "Bottom up step\n";
    #pragma omp parallel for reduction(vec_add: next_frontier) reduction(+: edges_frontier) schedule(auto)
    for (vidType i = 0; i < N; i++) {
      vidType start = rowptr[i];
      if (IS_VISITED(start) || is_unconnected(start)) {
        continue;
      }
      for (vidType j = start + 1; j < rowptr[i+1]; j++) {
        if (IS_VISITED(merged[j]) && copy_unmarked(merged[j]) == distance - 1) {
          // If neighbor is in frontier, add this vertex to next frontier
          add_to_frontier(next_frontier, start);
          set_distance(start, distance);
          break;
        }
      }
    }
  }

    void top_down_step(frontier this_frontier, frontier &next_frontier, weight_type distance) {
    // std::cout << "Top down step\n";
    // print merged
    #pragma omp parallel for reduction(vec_add: next_frontier) reduction(+: edges_frontier) schedule(auto) if (edges_frontier_old > 150)
    for (const auto &v : this_frontier) {
      for (vidType i = v + 1; !IS_MARKED(i); i++) {
        vidType neighbor = merged[i];
        if (!IS_VISITED(neighbor)) {
          add_to_frontier(next_frontier, neighbor);
          set_distance(neighbor, distance);
        }
      }
    }
  }


  void BFS(vidType source, weight_type *distances) {
    frontier this_frontier;
    vidType start = rowptr[source];
    dir = Direction::TOP_DOWN;
    add_to_frontier(this_frontier, start);
    set_distance(start, 0);
    weight_type distance = 1;
    while (!this_frontier.empty()) {
      // print_frontier(this_frontier);
      // std::cout << "Edges in frontier " << edges_frontier << ", vertices in frontier " << this_frontier.size();
      frontier next_frontier;
      next_frontier.reserve(this_frontier.size());
      if (dir == Direction::BOTTOM_UP && this_frontier.size() < N / BETA) {
        dir = Direction::TOP_DOWN;
      } else if (dir == Direction::TOP_DOWN && edges_frontier > unexplored_edges / ALPHA) {
        dir = Direction::BOTTOM_UP;
      }
      unexplored_edges -= edges_frontier;
      edges_frontier_old = edges_frontier;      
      edges_frontier = 0;
      if (dir == Direction::TOP_DOWN) {
        top_down_step(this_frontier, next_frontier, distance);
      } else {
        bottom_up_step(this_frontier, next_frontier, distance);
      }
      distance++;
      this_frontier = std::move(next_frontier);
    }
    compute_distances(distances, source);
  }
};

inline vidType get_degree(eidType *rowptr, vidType i) {
  return rowptr[i + 1] - rowptr[i];
}

void merged_csr(eidType *rowptr, vidType *col, vidType *merged, uint64_t N, uint64_t M) {
  vidType merged_index = 0;
  // Add degree of each vertex to the start of its neighbor list
  for (vidType i = 0; i < N; i++) {
    vidType start = rowptr[i];
    merged[merged_index++] = get_degree(rowptr, i) | MARKED_MASK;
    for (vidType j = start; j < rowptr[i + 1]; j++, merged_index++) {
      merged[merged_index] = rowptr[col[j]] + col[j];
    }
  }
  // Fix rowptr indices by adding offset caused by adding the degree to the start of
  // each neighbor list
  for (vidType i = 0; i <= N; i++) {
    rowptr[i] = rowptr[i] + i;
  }
  merged[M+N] = MARKED_MASK;
}

BaseGraph *initialize_graph(eidType *rowptr, vidType *col, uint64_t N,
                            uint64_t M) {
  vidType *merged = new vidType[M+N+1];
  merged_csr(rowptr, col, merged, N, M);
  return new Graph(rowptr, col, merged, N, M);
}

} // namespace bfs
