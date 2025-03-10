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
      : Gamestate(board, letter_values, letter_pool, {}) {};

  // Clears the letter from unlocked square p, if present.
  absl::Status ClearSquare(const Point &p);

  // Takes c from the remaining letters and places it in unlocked square p.
  absl::Status FillSquare(const Point &p, char c);

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
  std::vector<Point> MultiplierSquares() const;

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

  LetterCount letter_pool() const { return letter_pool_; }
  std::vector<std::string> letter_board() const { return letter_board_; }
  std::vector<std::vector<int>> mult_board() const { return mult_board_; }
  std::vector<std::vector<bool>> lock_board() const { return lock_board_; }
  absl::flat_hash_map<char, int> values() const { return values_; }
  std::vector<Point> bonus_path() const { return bonus_path_; }
  std::vector<Point> row_path(int row) const;
  std::string path_string(const std::vector<Point> &path) const;

  char char_at(const Point &p) const;
  char char_at(int row, int col) const;
  int multiplier_at(const Point &p) const;
  int multiplier_at(int row, int col) const;
  bool is_locked_at(const Point &p) const;
  bool is_locked_at(int row, int col) const;
  void set_is_locked_at(const Point &p, bool is_locked);
  void set_is_locked_at(int row, int col, bool is_locked);

 private:
  void set_char_at(const Point &p, char c);
  void set_char_at(int row, int col, char c);

  LetterCount letter_pool_;
  std::vector<std::string> letter_board_;
  std::vector<std::vector<int>> mult_board_;
  std::vector<std::vector<bool>> lock_board_;
  absl::flat_hash_map<char, int> values_;
  std::vector<Point> bonus_path_;

  /** * * * * * * * * *
   * Abseil functions *
   * * * * * * * * * **/

  // Allows hashing of BongoGameState.
  template <typename H>
  friend H AbslHashValue(H h, const Gamestate &bgs) {
    return H::combine(std::move(h), bgs.letter_pool_, bgs.letter_board_,
                      bgs.mult_board_, bgs.lock_board_, bgs.values_,
                      bgs.bonus_path_);
  }

  // Allows easy conversion of BongoGameState to string.
  template <typename Sink>
  friend void AbslStringify(Sink &sink, const Gamestate &bgs) {
    absl::Format(&sink, "%v\n[%s]\n[%s]\n[%s]\n[%s]\n[%s]", bgs.letter_pool_,
                 bgs.letter_board_[0], bgs.letter_board_[1],
                 bgs.letter_board_[2], bgs.letter_board_[3],
                 bgs.letter_board_[4]);
  }
};

bool operator==(const Gamestate &lhs, const Gamestate &rhs);
bool operator!=(const Gamestate &lhs, const Gamestate &rhs);

}  // namespace puzzmo::bongo

#endif