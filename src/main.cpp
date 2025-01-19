#include "input.hpp"
#include <omp.h>
#include <profiling.hpp>
#include <complete.hpp>

int main(const int argc, char **argv) {
  std::ifstream in("schemas/" + std::string(argv[1]) + ".json");
  nlohmann::json j;
  in >> j;
  quicktype::Inputschema data;
  quicktype::from_json(j, data);
  ProblemInput p = ProblemInput(data, complete::initialize_graph);
  LIKWID_MARKER_INIT;
  #pragma omp parallel
  {
    int num_threads = omp_get_num_threads();
    int thread_id = omp_get_thread_num();
    if (thread_id == 0) {
        printf("Number of threads: %d\n", num_threads);
    }
    LIKWID_MARKER_THREADINIT;
  }
  #pragma omp parallel
  {
    LIKWID_MARKER_START("bfs");
  }
  p.run();
  #pragma omp parallel
  {
    LIKWID_MARKER_STOP("bfs");
  }
  LIKWID_MARKER_CLOSE;
}