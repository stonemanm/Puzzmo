#ifndef spelltower_board_h
#define spelltower_board_h

#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "spelltower_path.h"
#include "src/shared/letter_count.h"
#include "src/shared/point.h"

namespace puzzmo {

// A board state for Spelltower.
class SpelltowerBoard {
 public:
  explicit SpelltowerBoard(const std::vector<std::string> &board);

  void ClearPoint(const Point &p);
  void ClearPath(const SpelltowerPath &path);
  absl::flat_hash_set<Point> AffectedSquares(const SpelltowerPath &path) const;

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
  std::string StarLetters() const;

  absl::flat_hash_set<std::string> GetAllStarRegexes() const;

  std::vector<LetterCount> GetRowLetterCounts() const;

  // Returns the coordinates of the bonus letters.
  std::vector<Point> StarLocations() const;

  // Calculates the score returned for the word along a given path.
  // Note: Score does not do any checking on the validity of points on the path.
  int ScorePath(const SpelltowerPath &path) const;

  /** * * * * * * * * * * *
   * Accessors & mutators *
   * * * * * * * * * * * **/

  // Returns the letter at a given spot on the board.
  char char_at(const Point &p) const;
  char char_at(int row, int col) const;

  std::vector<std::string> board() const;

  std::vector<Point> column(int col) const;
  std::vector<Point> row(int row) const;
  std::vector<Point> points() const;

  char &operator[](Point p);
  const char &operator[](Point p) const;
  std::string &operator[](int row);
  const std::string &operator[](int row) const;

  // Returns whether or not a given point has a star on it.
  bool is_star_at(const Point &p) const;
  bool is_star_at(int row, int col) const;

  absl::flat_hash_map<char, std::vector<Point>> letter_map() const {
    return letter_map_;
  }

 private:
  void RegenLetterMap();
  void JustClearPoint(const Point &p);

  std::vector<std::string> board_;
  int rows_, cols_ = 0;
  std::vector<Point> stars_;
  absl::flat_hash_map<char, std::vector<Point>> letter_map_;

  static const int max_cols_ = 13;
  static const int max_rows_ = 9;

  // Allows easy conversion of SpelltowerPath to string.
  template <typename Sink>
  friend void AbslStringify(Sink &sink, const SpelltowerBoard &board) {
    for (int row = 0; row < 9; ++row) {
      std::string row_string = board.board()[row];
      for (int col = 0; col < row_string.size(); ++col) {
        if (board.is_star_at(row, col))
          row_string[col] = std::toupper(row_string[col]);
      }
      sink.Append(absl::StrCat("\n", row, "[", row_string));
    }
  }
};

}  // namespace puzzmo

#endif