#include "graph.hpp"
#include <iostream>

bool BFS_Impl::check_distances(vidType source,
                               const weight_type *distances) const {
  Reference ref_input(graph);
  weight_type *ref_distances = new weight_type[graph->N];
  std::fill(ref_distances, ref_distances + graph->N, -1);
  ref_input.BFS(source, ref_distances);
  bool correct = true;
  for (int64_t i = 0; i < graph->N; i++) {
    if (distances[i] != ref_distances[i]) {
      std::cout << "Incorrect value for vertex " << i
                << ", expected distance " + std::to_string(ref_distances[i]) +
                       ", but got " + std::to_string(distances[i]) + "\n";
      correct = false;
    }
  }
  return correct;
}

bool BFS_Impl::check_parents(vidType source, const weight_type *parents) const {
  bool correct = true;
  std::vector<vidType> depth(graph->N, -1);
  std::vector<vidType> to_visit;
  depth[source] = 0;
  to_visit.push_back(source);
  to_visit.reserve(graph->N);
  // Run BFS to compute depth of each vertex
  for (auto it = to_visit.begin(); it != to_visit.end(); it++) {
    vidType i = *it;
    for (int64_t v = graph->rowptr[i]; v < graph->rowptr[i + 1]; v++) {
      if (depth[graph->col[v]] == -1) {
        depth[graph->col[v]] = depth[i] + 1;
        to_visit.push_back(graph->col[v]);
      }
    }
  }
  for (int64_t i = 0; i < graph->N; i++) {
    // Check if vertex is part of the BFS tree
    if (depth[i] != -1 && parents[i] != -1) {
      // Check if parent is correct
      if (i == source) {
        if (!((parents[i] == i) && (depth[i] == 0))) {
          std::cout << "Source wrong";
          correct = false;
        }
        continue;
      }
      bool parent_found = false;
      for (int64_t j = graph->rowptr[i]; j < graph->rowptr[i + 1]; j++) {
        if (graph->col[j] == parents[i]) {
          vidType parent = graph->col[j];
          // Check if parent has correct depth
          if (depth[parent] != depth[i] - 1) {
            std::cout << "Wrong depth of child " + std::to_string(i) +
                             " (parent " + std::to_string(parent) +
                             " with depth " + std::to_string(depth[parent])
                      << ")" << std::endl;
            correct = false;
          }
          parent_found = true;
          break;
        }
      }
      if (!parent_found) {
        std::cout << "Couldn't find edge from " << parents[i] << " to "
                  << std::to_string(i) << std::endl;
        correct = false;
      }
      // Check if parent = -1 and parent = -1
    } else if (depth[i] != parents[i]) {
      std::cout << "Reachability mismatch" << std::endl;
      correct = false;
    }
  }
  return correct;
}