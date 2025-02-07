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
  char At(const Point &p) const;
  char At(int row, int col) const;

  // Returns true if the point exists on the board.
  bool HasPoint(const Point &p) const;
  bool HasPoint(int row, int col) const;

  // Returns the subset of `p`'s von Neumann neighbors that are on the board.
  absl::flat_hash_set<Point> ValidVonNeumannNeighbors(const Point &p) const;

  // Returns the subset of `p`'s Moore neighbors that are on the board.
  absl::flat_hash_set<Point> ValidMooreNeighbors(const Point &p) const;

  // Returns the number of rows, columns, or stars on the board.
  int NumRows() const;
  int NumCols() const;
  int NumStars() const;

  absl::flat_hash_set<std::string> GetAllStarRegexes() const;

  // Returns the coordinates of the bonus letters.
  std::vector<Point> StarLocations() const;

  // Calculates the score returned for the word along a given path.
  // Note: Score does not do any checking on the validity of points on the path.
  int Score(const absl::flat_hash_set<Point> &path) const;

  // Checks the rows (columns on the main board) to see if the letters of `word`
  // occur in adjacent rows.
  bool MightHaveWord(const std::string &word) const;
  bool MightHaveWord(const std::string &word, bool all_star) const;

  std::vector<std::string>
  MightHaveWords(const std::vector<std::string> &words) const;
  std::vector<std::string> MightHaveWords(const std::vector<std::string> &words,
                                          bool all_star) const;

private:
  std::vector<std::vector<char>> board_;
  int rows_, cols_;
  std::vector<Point> stars_;

  bool DFS(const std::string &word, int i, int row) const;
  bool DFS(const std::string &word, int i, int row,
           std::vector<bool> &used_stars) const;
};

} // namespace puzzmo

#endif