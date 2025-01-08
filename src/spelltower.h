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

using WordMap =
    absl::btree_map<int, absl::btree_set<std::string>, std::greater<int>>;

namespace puzzmo {

const int kLetterScores[] = {
    /* a = */ 1, /* b = */ 4,  /* c = */ 4, /* d = */ 3, /* e = */ 1,
    /* f = */ 5, /* g = */ 3,  /* h = */ 5, /* i = */ 1, /* j = */ 9,
    /* k = */ 6, /* l = */ 2,  /* m = */ 4, /* n = */ 2, /* o = */ 1,
    /* p = */ 4, /* q = */ 12, /* r = */ 2, /* s = */ 1, /* t = */ 2,
    /* u = */ 1, /* v = */ 5,  /* w = */ 5, /* x = */ 9, /* y = */ 5,
    /* z = */ 11};

struct Point {
  int i;
  int j;

  bool operator==(const Point &other) const {
    return i == other.i && j == other.j;
  }

  Point operator+(const Point &other) const {
    return {i + other.i, j + other.j};
  }

  template <typename H> friend H AbslHashValue(H h, const Point &p) {
    return H::combine(std::move(h), p.i, p.j);
  }
};

const Point kAdjacentCoords[] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
                                 {0, 1},   {1, -1}, {1, 0},  {1, 1}};

const Point kDPad[] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

struct TrieNode {
  std::vector<std::shared_ptr<TrieNode>> children;
  const std::string *word = nullptr;
  TrieNode() : children(26) {}
};

class Spelltower {
public:
  Spelltower(std::vector<std::vector<char>> &board)
      : board_(board), n_(board.size()), m_(board[0].size()),
        root_(std::make_shared<TrieNode>()) {};

  // Add one or more words to the trie
  // void AddToDictionary(const std::string &word) { AddToDictionary({word}); }
  void AddToDictionary(const std::vector<std::string> words);

  const WordMap FindWords();

  const char LetterAt(Point p);

private:
  std::vector<std::vector<char>> board_;
  const int n_, m_;
  const std::shared_ptr<TrieNode> root_;

  void DFS(std::vector<std::vector<bool>> &visited, Point p,
           std::shared_ptr<TrieNode> node, absl::flat_hash_set<Point> &path,
           WordMap &ans);

  int Score(const absl::flat_hash_set<Point> &path);

  bool Valid(int i, int j);
  bool Valid(const Point c);
};

} // namespace puzzmo

#endif
