#ifndef spelltower_board_h
#define spelltower_board_h

#include <string>
#include <vector>

#include "absl/container/flat_hash_set.h"

#include "point.h"

namespace puzzmo {

// The value of each letter, in an array. Indexed by `c - 'a'`.
const int kLetterScores[] = {
    /* a = */ 1, /* b = */ 4,  /* c = */ 4, /* d = */ 3, /* e = */ 1,
    /* f = */ 5, /* g = */ 3,  /* h = */ 5, /* i = */ 1, /* j = */ 9,
    /* k = */ 6, /* l = */ 2,  /* m = */ 4, /* n = */ 2, /* o = */ 1,
    /* p = */ 4, /* q = */ 12, /* r = */ 2, /* s = */ 1, /* t = */ 2,
    /* u = */ 1, /* v = */ 5,  /* w = */ 5, /* x = */ 9, /* y = */ 5,
    /* z = */ 11};

// A board state for Spelltower.
class SpelltowerBoard {
public:
  explicit SpelltowerBoard(const std::vector<std::vector<char>> &board);

  // Returns the letter at a given spot on the board.
  char LetterAt(const Point &p) const;

  // Returns true if the point exists on the board.
  bool HasPoint(const Point &p) const;
  bool HasPoint(int row, int col) const;

  // Returns the number of rows or columns on the board.
  int NumRows() const;
  int NumCols() const;

  // Calculates the score returned for the word along a given path.
  int Score(const absl::flat_hash_set<Point> &path) const;

private:
  std::vector<std::vector<char>> board_;
  int rows_, cols_;
};

} // namespace puzzmo

#endif