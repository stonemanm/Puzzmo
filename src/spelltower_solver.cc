#include "spelltower_solver.h"

#include <cctype>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "absl/container/flat_hash_set.h"

namespace puzzmo {

using WordMap =
    absl::btree_map<int, absl::btree_set<std::string>, std::greater<int>>;

void SpelltowerSolver::DFS(std::shared_ptr<TrieNode> node, const Point &p,
                           const SpelltowerBoard &board,
                           std::vector<std::vector<bool>> &visited,
                           absl::flat_hash_set<Point> &path, WordMap &ans) {
  if (!board.HasPoint(p) || visited[p.row][p.col] || board.At(p) == '*' ||
      board.At(p) == ' ') {
    return;
  }

  const char c = std::tolower(board.At(p));
  std::shared_ptr<TrieNode> child = node->children[c - 'a'];
  if (child == nullptr) {
    return;
  }

  path.insert(p);
  visited[p.row][p.col] = true;

  if (child->word != nullptr) {
    int s = board.Score(path);
    ans[s].insert(*child->word);
  }

  for (const Point &neighbor : board.ValidMooreNeighbors(p)) {
    DFS(child, neighbor, board, visited, path, ans);
  }

  path.erase(p);
  visited[p.row][p.col] = false;
}

const WordMap SpelltowerSolver::FindWords() {
  absl::flat_hash_set<Point> path;
  WordMap words;
  int rows = starting_board_.NumRows();
  int cols = starting_board_.NumCols();
  std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols));
  for (int row = 0; row < rows; ++row) {
    for (int col = 0; col < cols; ++col) {
      DFS(dict_, {.row = row, .col = col}, starting_board_, visited, path,
          words);
    }
  }
  return words;
}

} // namespace puzzmo