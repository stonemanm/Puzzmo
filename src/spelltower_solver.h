#ifndef spelltower_solver_h
#define spelltower_solver_h

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "absl/container/btree_map.h"
#include "absl/container/btree_set.h"
#include "absl/container/flat_hash_set.h"

#include "dictionary_utils.h"
#include "point.h"
#include "spelltower_board.h"

namespace puzzmo {

class SpelltowerSolver {
public:
  SpelltowerSolver(SpelltowerBoard &board, const std::shared_ptr<TrieNode> dict)
      : starting_board_(board), dict_(dict) {};

  // Returns a map of all valid words on the board, as well as their score
  const absl::btree_map<int, absl::btree_set<std::string>, std::greater<int>>
  FindWords();

private:
  SpelltowerBoard starting_board_;
  const std::shared_ptr<TrieNode> dict_;

  void DFS(std::shared_ptr<TrieNode> node, const Point &p,
           const SpelltowerBoard &board,
           std::vector<std::vector<bool>> &visited,
           absl::flat_hash_set<Point> &path,
           absl::btree_map<int, absl::btree_set<std::string>, std::greater<int>>
               &ans);
};

} // namespace puzzmo

#endif
