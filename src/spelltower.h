#ifndef spelltower_h
#define spelltower_h

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

using WordMap =
    absl::btree_map<int, absl::btree_set<std::string>, std::greater<int>>;

namespace puzzmo {

class Spelltower {
public:
  Spelltower(std::vector<std::vector<char>> &board,
             const std::shared_ptr<TrieNode> dict)
      : board_(board), n_(board.size()), m_(board[0].size()), dict_(dict) {};

  const WordMap FindWords();

  const char LetterAt(Point p);

private:
  std::vector<std::vector<char>> board_;
  const int n_, m_;
  const std::shared_ptr<TrieNode> dict_;

  void DFS(std::vector<std::vector<bool>> &visited, Point p,
           std::shared_ptr<TrieNode> node, absl::flat_hash_set<Point> &path,
           WordMap &ans);

  int Score(const absl::flat_hash_set<Point> &path);

  bool Valid(int i, int j);
  bool Valid(const Point p);
};

} // namespace puzzmo

#endif
