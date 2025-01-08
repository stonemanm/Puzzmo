#ifndef spelltower_board_h
#define spelltower_board_h

#include <string>
#include <vector>

#include "point.h"

namespace puzzmo {

const int kLetterScores[] = {
    /* a = */ 1, /* b = */ 4,  /* c = */ 4, /* d = */ 3, /* e = */ 1,
    /* f = */ 5, /* g = */ 3,  /* h = */ 5, /* i = */ 1, /* j = */ 9,
    /* k = */ 6, /* l = */ 2,  /* m = */ 4, /* n = */ 2, /* o = */ 1,
    /* p = */ 4, /* q = */ 12, /* r = */ 2, /* s = */ 1, /* t = */ 2,
    /* u = */ 1, /* v = */ 5,  /* w = */ 5, /* x = */ 9, /* y = */ 5,
    /* z = */ 11};

class SpelltowerBoard {
public:
  explicit SpelltowerBoard(const std::vector<std::vector<char>> &board)
      : board_(board), rows_(board.size()), cols_(board.at(0).size()) {};

  const char LetterAt(Point p);

private:
  std::vector<std::vector<char>> board_;
  int rows_, cols_;
};

} // namespace puzzmo

#endif