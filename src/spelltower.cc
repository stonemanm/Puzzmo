#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "spelltower.h"

namespace puzzmo {

void Spelltower::DFS(std::shared_ptr<TrieNode> node, const Point &p,
                     const SpelltowerBoard &board,
                     std::vector<std::vector<bool>> &visited,
                     absl::flat_hash_set<Point> &path, WordMap &ans) {
  if (!board.HasPoint(p) || visited[p.row][p.col] || board.LetterAt(p) == '*' ||
      board.LetterAt(p) == ' ') {
    return;
  }

  const char c = board.LetterAt(p);
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

  for (const auto &dp : kAdjacentCoords) {
    DFS(child, p + dp, board, visited, path, ans);
  }

  path.erase(p);
  visited[p.row][p.col] = false;
}

const WordMap Spelltower::FindWords() {
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