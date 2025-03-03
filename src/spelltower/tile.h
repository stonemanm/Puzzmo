// -----------------------------------------------------------------------------
// File: tile.h
// -----------------------------------------------------------------------------
//
// This header file defines tiles: the atomic pieces that make up a Spelltower
// game. No two tiles on a grid will have the same coordinates, although the
// specific coordinates may shift as words are cleared. A tile can have a single
// letter or be blank. Some tiles are also star tiles, which increase the score
// multiplier when clearing a word.

#ifndef tile_h
#define tile_h

#include <cctype>

#include "absl/status/status.h"
#include "src/shared/point.h"

namespace puzzmo::spelltower {

constexpr char kBlankTileLetter = '.';
constexpr char kRareTileLetters[] = {'j', 'q', 'x', 'z'};

class Tile {
 public:
  Tile(int row, int col) : Tile(row, col, kBlankTileLetter, false) {}
  Tile(int row, int col, char letter)
      : Tile(row, col, std::tolower(letter), std::isupper(letter)) {}
  Tile(int row, int col, char letter, bool is_star);

  //-----------
  // Accessors

  // Return the coordinates at which this tile can be found in the grid.
  Point coords() const { return coords_; }
  int row() const { return coords_.row; }
  int col() const { return coords_.col; }

  // Returns the letter on this tile. If tile is blank, returns an asterisk. If
  // tile is empty, returns a space.
  char letter() const { return letter_; }

  bool is_blank() const { return letter_ == kBlankTileLetter; }
  bool is_letter() const { return std::isalpha(letter_); }
  bool is_rare() const { return absl::c_contains(kRareTileLetters, letter_); }
  bool is_star() const { return is_star_; }

  //-----------
  // Mutators

  // Lowers the tile's row. Returns an OutOfRangeError if this would reduce
  // `coords_.row` below 0.
  absl::Status Drop(int rows);

  void set_is_star(bool is_star) { is_star_ = is_star; }
  void set_letter(char letter) { letter_ = letter; }

 private:
  Point coords_;
  char letter_;
  bool is_star_;

  //------------------
  // Abseil functions

  template <typename H>
  friend H AbslHashValue(H h, const Tile &tile) {
    return H::combine(std::move(h), tile.coords_, tile.letter_, tile.is_star_);
  }

  template <typename Sink>
  friend void AbslStringify(Sink &sink, const Tile &tile) {
    sink.Append(std::string(
        1, tile.is_star_ ? std::toupper(tile.letter_) : tile.letter_));
  }
};

bool operator==(const Tile &lhs, const Tile &rhs);
bool operator!=(const Tile &lhs, const Tile &rhs);

}  // namespace puzzmo::spelltower

#endif  // !tile_h