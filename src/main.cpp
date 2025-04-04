#include "graph.hpp"
#include <algorithm>
#include <limits>
#include <omp.h>
#include <string>

#define USAGE                                                                  \
  "Usage: %s <schema> <source> <implementation> <check>\nRuns BFS "            \
  "implementations. \n\nMandatory arguments:\n  <schema>\t path to JSON "      \
  "schema of dataset \n  <source>\t : integer. Source vertex ID "              \
  "('0' by default) \n  <algorithm>\t : 'merged_csr_parents', 'merged_csr', "  \
  "'bitmap', 'classic', 'reference', 'heuristic' ('heuristic' by default) \n " \
  " <check>\t : 'true', false'. Checks correctness of the result ('false' by " \
  "default)\n"

BFS_Impl *initialize_BFS(std::string filename, std::string algo_str) {
  std::string path = "schemas/" + filename;
  Graph *graph = new Graph(path);

  if (algo_str == "merged_csr_parents") {
    return new MergedCSR_Parents(graph);
  } else if (algo_str == "merged_csr") {
    return new MergedCSR(graph);
  } else if (algo_str == "bitmap") {
    return new Bitmap(graph);
  } else if (algo_str == "classic") {
    return new Classic(graph);
  } else if (algo_str == "reference") {
    return new Reference(graph);
  } else {
    if ((float)(graph->M) / graph->N < 10) { // Graph diameter heuristic
      return new MergedCSR(graph);
    } else {
      return new Bitmap(graph);
    }
  }
}

int main(const int argc, char **argv) {
  if (argc < 2 || argc > 5) {
    printf(USAGE, argv[0]);
    return 1;
  }
  vidType source = 0;
  bool check = false;

  if (argc > 2) {
    source = std::stoi(argv[2]);
  }
  if (argc > 4) {
    std::string check_str = argv[4];
    if (check_str == "true") {
      check = true;
    }
  }

#pragma omp parallel
  {
#pragma omp master
    { printf("Number of threads: %d\n", omp_get_num_threads()); }
  }
  double t_start = omp_get_wtime();
  BFS_Impl *bfs = initialize_BFS(std::string(argv[1]), std::string(argv[3]));
  double t_end = omp_get_wtime();

  printf("Initialization: %f\n", t_end - t_start);

  weight_type *result = new weight_type[bfs->graph->N];
  // Initialize result vector
  std::fill_n(result, bfs->graph->N, std::numeric_limits<weight_type>::max());

  t_start = omp_get_wtime();
  bfs->BFS(source, result);
  t_end = omp_get_wtime();

  printf("Runtime: %f\n", t_end - t_start);

  if (check) {
    bfs->check_result(source, result);
  }
}