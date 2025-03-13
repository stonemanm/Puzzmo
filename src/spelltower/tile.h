// -----------------------------------------------------------------------------
// File: tile.h
// -----------------------------------------------------------------------------
//
// This header file defines tiles: the atomic pieces that make up a Spelltower
// game. No two tiles on a grid will have the same coordinates, although the
// specific coordinates may shift as words are cleared. A tile can have a single
// letter or be blank. Some tiles are also star tiles, which increase the score
// multiplier when clearing a word.

#ifndef PUZZMO_SPELLTOWER_TILE_H_
#define PUZZMO_SPELLTOWER_TILE_H_

#include <cctype>

#include "absl/status/status.h"
#include "src/shared/point.h"

namespace puzzmo::spelltower {

// The character that will be contained by a blank Tile, regardless of what
// alphanumeric character was initially passed to the constructor.
//
// Prefer calling `Tile::is_blank()` rather than using in a direct comparison.
constexpr char kBlankTileLetter = '.';

// The letters that make a tile a rare tile. Rare tiles are highlighted in red
// on the Spelltower board, and clear their whole row when used in a word.
//
// As with `kBlankTileLetter`, prefer using `Tile::is_rare()` rather than this.
constexpr char kRareTileLetters[] = {'j', 'q', 'x', 'z'};

// spelltower::Tile
//
// The `Tile` class represents a piece of the Spelltower board: a letter (or
// blank tile), a star (if one is present), and the row and column at which the
// tile can be found on the board. Tiles will never change columns, but can fall
// rows if the tiles beneath them are removed.
class Tile {
 public:
  //--------------
  // Constructors

  // If initialized with only a row and column, Tile will default to being
  // blank.
  explicit Tile(const Point &p) : Tile(p.row, p.col) {}
  Tile(int row, int col) : Tile(row, col, kBlankTileLetter) {}

  // If also passed a letter, that letter will be stored. If the letter is
  // capitalized, the lowercase letter will be scored, but the tile created will
  // be a star tile. If passed a non-alphabetical character, a blank tile will
  // be created.
  Tile(const Point &p, char letter) : Tile(p.row, p.col, letter) {}
  Tile(int row, int col, char letter);

  //-----------
  // Accessors

  // Tile::coords()
  //
  // Return the coordinates at which this tile can be found in the grid, as
  // represented by a `Point`.
  Point coords() const { return coords_; }

  // Tile::is_on_grid()
  //
  // Proides access to the underlying data member.
  bool is_on_grid() const { return is_on_grid_; }

  // Tile::row()
  //
  // A shortcut to `Tile::coords().row`.
  int row() const { return coords_.row; }

  // Tile::col()
  //
  // A shortcut to `Tile::coords().col`.
  int col() const { return coords_.col; }

  // Tile::letter()
  //
  // Returns the character on this tile. If tile is blank, returns
  // `kBlankTileLetter`.
  char letter() const { return letter_; }

  // Tile::letter_on_board()
  //
  // Returns the character on this tile, capitalized if it is a star tile and
  // converted to a length-1 string instead of a char.
  std::string letter_on_board() const {
    return std::string(1, is_star_ ? std::toupper(letter_) : letter_);
  }

  // Tile::is_blank()
  //
  // Returns `true` if `letter_` is not an alphabetical character. Prefer
  // checking for a blank tile using this method.
  bool is_blank() const { return letter_ == kBlankTileLetter; }

  // Tile::is_rare()
  //
  // Returns `true` if `letter_` is a rare letter. Rare tiles clear their entire
  // row when used in a word.
  bool is_rare() const { return absl::c_contains(kRareTileLetters, letter_); }

  // Tile::is_star()
  //
  // Returns `true` if the tile is a star tile. When a word is scored, the final
  // score is multiplied by (1 + #_of_star_tiles_in_that_word).
  bool is_star() const { return is_star_; }

  // Tile::value()
  //
  // Returns the value of the tile when calculating the score. The value of each
  // letter is hardcoded into the game data.
  int value() const { return value_; }

  //-----------
  // Mutators

  // Tile::set_is_on_grid()
  //
  // A simple mutator for `is_on_grid_`.
  void set_is_on_grid(bool is_on_grid) { is_on_grid_ = is_on_grid; }

  // Tile::Drop()
  //
  // Lowers the tile's row. Returns an OutOfRangeError if this would reduce
  // `coords_.row` below 0.
  absl::Status Drop(int rows);

  //---------
  // Members

 private:
  Point coords_;
  bool is_on_grid_;
  const char letter_;
  const bool is_star_;
  const int value_;

  //------------------
  // Abseil functions

  template <typename H>
  friend H AbslHashValue(H h, const Tile &tile) {
    return H::combine(std::move(h), tile.coords_, tile.letter_, tile.is_star_);
  }

  template <typename Sink>
  friend void AbslStringify(Sink &sink, const Tile &tile) {
    absl::Format(&sink, "%s %v", tile.letter_on_board(), tile.coords());
  }
};

bool operator==(const Tile &lhs, const Tile &rhs);
bool operator!=(const Tile &lhs, const Tile &rhs);

}  // namespace puzzmo::spelltower

#endif