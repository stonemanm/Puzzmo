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
// The `Gamestate` class represents the state of play in a Bongo game at any
// given moment. Most of the information is defined within two data members: a
// 5x5 matrix of `Cell` objects and a `LetterCount` of unplayed letters. It also
// contains a vector of `Point` objects charting the location of the bonus line,
// as well as a map of the values for each letter when scoring.
class Gamestate {
 public:
  //--------------
  // Constructors

  // Creates a `Gamestate` object from a vector of strings representing the
  // board, a `LetterCount` representing the unplaced tiles, a map of letter
  // values, and another vector of strings indicating already-placed letters.
  // Both `board` and `letter_board` must be 5x5.
  Gamestate(const std::vector<std::string> grid_strings,
            absl::flat_hash_map<char, int> letter_values, LetterCount letters,
            std::vector<std::string> placed_letters);

  // A simpler constructor for the case when no letters have been placed.
  // `board` must be 5x5.
  Gamestate(const std::vector<std::string> grid_strings,
            absl::flat_hash_map<char, int> letter_values, LetterCount letters)
      : Gamestate(grid_strings, letter_values, letters,
                  std::vector<std::string>(5, std::string(5, kEmptyCell))) {};

  //-----------
  // Accessors

  // Gamestate::grid()
  //
  // Provides access to the underlying matrix of `Cell`s.
  std::vector<std::vector<Cell>> grid() const { return grid_; }

  // operator[]
  //
  // `Gamestate` has two separate overloaded subscript operators. If provided an
  // int, it treats it as the index of a row and returns that row. If provided
  // a `Point` instead, it returns the `Cell` at that row and column in `grid_`.
  std::vector<Cell> &operator[](int row) { return grid_[row]; }
  const std::vector<Cell> &operator[](int row) const { return grid_[row]; }
  Cell &operator[](Point p) { return grid_[p.row][p.col]; }
  const Cell &operator[](Point p) const { return grid_[p.row][p.col]; }

  // Gamestate::MultiplierPoints()
  //
  // Returns a vector of points to cells with multipliers greater than 1.
  std::vector<Point> MultiplierPoints() const;

  // Gamestate::DoublePoints()
  //
  // Returns a vector of points to cells with 2x multipliers.
  std::vector<Point> DoublePoints() const;

  // Gamestate::TriplePoint()
  //
  // Returns a point to the cell with a 3x multiplier.
  Point TriplePoint() const;

  // Gamestate::letters()
  //
  // Returns a `LetterCount` of all letters in the gamestate, both placed and
  // unplaced.
  const LetterCount letters() const { return letters_; }

  // Gamestate::placed_letters()
  //
  // Returns a `LetterCount` with all of the letters that have been placed on
  // the board.
  LetterCount placed_letters() const { return letters_ - unplaced_letters_; }

  // Gamestate::unplaced_letters()
  //
  // Provides access to a `LetterCount` of the unplaced letters.
  LetterCount unplaced_letters() const { return unplaced_letters_; }

  // Gamestate::NMostValuableLetters()
  //
  // Returns the n letters in `unplaced_letters_` with the highest values,
  // sorted from highest value to lowest. If fewer than n letters remain,
  // returns all of them.
  std::string NMostValuableLetters(int n) const;

  // Gamestate::line()
  //
  // Returns a vector of points corresponding to the line in the given row.
  std::vector<Point> line(int row) const;

  // Gamestate::bonus_line()
  //
  // Returns a vector of points corresponding to the bonus line on the grid.
  std::vector<Point> bonus_line() const { return bonus_line_; }

  // Gamestate::LineRegex()
  //
  // Returns regex to match the line as it currently exists. Any letters will
  // match, and any nonalphabetical characters will be replaced with regex
  // matching any character in `unplaced_letters_`.
  std::string LineRegex(const std::vector<Point> &line) const;

  // Gamestate::LineString()
  //
  // Returns a string comprised of the letter in each `Cell` pointed to by the
  // `Point` in `line`.
  std::string LineString(const std::vector<Point> &line) const;

  // Gamestate::letter_values()
  //
  // Provides access to a map from chars to their associated tile score.
  absl::flat_hash_map<char, int> letter_values() const {
    return letter_values_;
  }

  //----------
  // Mutators

  // Gamestate::ClearCell()
  //
  // Clears the letter from unlocked cell p, if present.
  absl::Status ClearCell(const Point &p);

  // Gamestate::FillCell()
  //
  // Takes c from the remaining letters and places it in unlocked cell p.
  absl::Status FillCell(const Point &p, char c);

  // Gamestate::ClearLine()
  //
  // Clears all unlocked letters from cells along the line.
  absl::Status ClearLine(const std::vector<Point> &line);

  // Gamestate::FillLine()
  //
  // Fills cells along the line with `word`, unless it conflicts with the
  // contents of a locked cell.
  absl::Status FillLine(const std::vector<Point> &line, absl::string_view word);

  // Gamestate::ClearBoard()
  //
  // Clears all unlocked letters from the board.
  absl::Status ClearBoard();

  //------------
  // Relational

  // Gamestate::IsChildOf()
  //
  // Returns `true` if `other` could have come from the same starting state and
  // this gamestate could be turned into `other` solely by placing letters into
  // empty cells.
  bool IsChildOf(const Gamestate &other) const;

  //---------
  // Scoring

  // Gamestate::LinesToScore()
  //
  // Returns the six lines used to score the board.
  std::vector<std::vector<Point>> LinesToScore() const;

  // Gamestate::UpperBoundOnScore()
  //
  // Returns the highest possible score for the gamestate with the assumption
  // that every string is a common word.
  int UpperBoundOnScore() const;

 private:
  std::vector<std::vector<Cell>> grid_;
  LetterCount letters_;
  LetterCount unplaced_letters_;
  absl::flat_hash_map<char, int> letter_values_;
  std::vector<Point> bonus_line_;

  //------------------
  // Abseil functions

  template <typename H>
  friend H AbslHashValue(H h, const Gamestate &bgs) {
    return H::combine(std::move(h), bgs.grid_, bgs.unplaced_letters_,
                      bgs.letter_values_, bgs.bonus_line_);
  }

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