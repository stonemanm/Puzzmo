// -----------------------------------------------------------------------------
// File: gamestate.h
// -----------------------------------------------------------------------------
//
// This header file defines the gamestate of a Bongo puzzle at any given moment.
// It maintains the locations of the bonus line and multiplier cells in the
// grid, the positions of any tiles that have been placed, and the quantity and
// value of different letter tiles available.

#ifndef PUZZMO_BONGO_GAMESTATE_H_
#define PUZZMO_BONGO_GAMESTATE_H_

#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "src/shared/letter_count.h"
#include "src/shared/point.h"

namespace puzzmo::bongo {

constexpr char kBonusCell = '*';
// The character used for an empty cell.
constexpr char kEmptyCell = '_';
constexpr char kDoubleMultiplier = '2';
constexpr char kTripleMultiplier = '3';

// bongo::Cell
//
// The `Cell` struct is an atomic component of a Bongo grid. It has a multiplier
// value of 1x, 2x, or 3x, and may or may not contain a letter. It can also be
// locked, which prevents it from being filled or cleared.
struct Cell {
  bool is_locked = false;
  char letter = kEmptyCell;
  int multiplier = 1;
};

bool operator==(const Cell &lhs, const Cell &rhs);
bool operator!=(const Cell &lhs, const Cell &rhs);

// bongo::Gamestate
//
//
class Gamestate {
 public:
  // board and letter_grid should be vector of length 5, containing 5-long
  // strings. If vectors or strings have length > 5, only the first 5 will be
  // saved. If vectors or strings have length < 5, empty spaces will fill what
  // remains.
  Gamestate(const std::vector<std::string> board,
            absl::flat_hash_map<char, int> letter_values,
            LetterCount letter_pool, std::vector<std::string> letter_board);
  Gamestate(const std::vector<std::string> board,
            absl::flat_hash_map<char, int> letter_values,
            LetterCount letter_pool)
      : Gamestate(board, letter_values, letter_pool,
                  std::vector<std::string>(5, std::string(5, kEmptyCell))) {};

  // Clears the letter from unlocked square p, if present.
  absl::Status ClearCell(const Point &p);

  // Takes c from the remaining letters and places it in unlocked square p.
  absl::Status FillCell(const Point &p, char c);

  // Clears all unlocked letters from squares along the path.
  absl::Status ClearPath(const std::vector<Point> &path);

  // Fills squares along the path with the string_view, unless it conflicts with
  // the contents of a locked square.
  absl::Status FillPath(const std::vector<Point> &path, absl::string_view sv);

  // Clears all unlocked letters from the board.
  absl::Status ClearBoard();

  // Returns the six paths used to score the board.
  std::vector<std::vector<Point>> PathsToScore() const;

  // Grabs the longest consecutive substring of letters from the path. If it is
  // 3+ characters (or 4+ if it's the bonus path), return it; otherwise, returns
  // an empty string. Note that whether or not this is a dictionary word is not
  // validated.
  std::string GetWord(const std::vector<Point> &path) const;

  // Returns true iff `other` could have come from the same starting state and
  // has placed a subset of the letters that this gamestate has. (On the
  // same squares, of course.)
  bool IsChildOf(const Gamestate &other) const;

  // Returns true iff every row has a word.
  bool IsComplete() const;

  // Returns the index of the row with the most letters but doesn't have 3+
  // consecutive letters. Breaks ties in favor of the lowest index.
  int MostRestrictedWordlessRow() const;

  LetterCount AllLetters() const;
  int NumLetters() const;
  int NumLettersLeft() const;
  int NumLettersPlaced() const;

  // Returns a vector of the points that have tile multipliers on them.
  std::vector<Point> MultiplierCells() const;

  // Returns the n remaining letters with the highest scores, with the highest
  // values first. If fewer than n letters remain, returns all of them.
  std::string NMostValuableTiles(int n) const;

  // Returns regex to match the path as it currently exists. Any letters will
  // match, and any nonalphabetical characters are replaced with regex matching
  // any of the letters remaining.
  std::string RegexForPath(const std::vector<Point> &path) const;

  /** * * * * * * * * * * *
   * Accessors & mutators *
   * * * * * * * * * * * **/

  std::vector<std::vector<Cell>> grid() const { return grid_; }
  LetterCount letter_pool() const { return letter_pool_; }
  absl::flat_hash_map<char, int> values() const { return values_; }
  std::vector<Point> bonus_path() const { return bonus_line_; }
  std::vector<Point> row_path(int row) const;
  std::string path_string(const std::vector<Point> &path) const;

  // operator[]
  //
  // `Gamestate` has two separate overloaded subscript operators. If provided an
  // int, it treats it as the index of a row and returns that row. If provided
  // a `Point` instead, it returns the `Cell` at that row and column in `grid_`.
  std::vector<Cell> &operator[](int row) { return grid_[row]; }
  const std::vector<Cell> &operator[](int row) const { return grid_[row]; }
  Cell &operator[](Point p) { return grid_[p.row][p.col]; }
  const Cell &operator[](Point p) const { return grid_[p.row][p.col]; }

 private:
  std::vector<std::vector<Cell>> grid_;
  LetterCount letter_pool_;
  absl::flat_hash_map<char, int> values_;
  std::vector<Point> bonus_line_;

  /** * * * * * * * * *
   * Abseil functions *
   * * * * * * * * * **/

  // Allows hashing of BongoGameState.
  template <typename H>
  friend H AbslHashValue(H h, const Gamestate &bgs) {
    return H::combine(std::move(h), bgs.grid_, bgs.letter_pool_, bgs.values_,
                      bgs.bonus_line_);
  }

  // Allows easy conversion of BongoGameState to string.
  template <typename Sink>
  friend void AbslStringify(Sink &sink, const Gamestate &bgs) {
    sink.Append(absl::StrJoin(
        bgs.grid_, "\n", [](std::string *out, const std::vector<Cell> &row) {
          absl::StrAppend(out, "[",
                          absl::StrJoin(row, "",
                                        [](std::string *out, const Cell &cell) {
                                          absl::StrAppend(
                                              out, std::string(1, cell.letter));
                                        }),
                          "]");
        }));
  }
};

bool operator==(const Gamestate &lhs, const Gamestate &rhs);
bool operator!=(const Gamestate &lhs, const Gamestate &rhs);

}  // namespace puzzmo::bongo

#endif