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
  Tile(int row, int col)
      : coords_({.row = row, .col = col}),
        letter_(kBlankTileLetter),
        is_star_(false) {}

  // If also passed a letter, that letter will be stored. If the letter is
  // capitalized, the lowercase letter will be scored, but the tile created will
  // be a star tile. If passed a non-alphabetical character, a blank tile will
  // be created.
  Tile(int row, int col, char letter)
      : coords_({.row = row, .col = col}),
        letter_(std::isalpha(letter) ? std::tolower(letter) : kBlankTileLetter),
        is_star_(std::isalpha(letter)) {}

  //-----------
  // Accessors

  // Tile::coords()
  //
  // Return the coordinates at which this tile can be found in the grid, as
  // represented by a `Point`.
  Point coords() const { return coords_; }

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

  //-----------
  // Mutators

  // Tile::Drop()
  //
  // Lowers the tile's row. Returns an OutOfRangeError if this would reduce
  // `coords_.row` below 0.
  absl::Status Drop(int rows);

  // Tile::set_is_star()
  //
  // Modifies the data member `is_star_`. Should never need use after
  // construction, save for in testing.
  void set_is_star(bool is_star) { is_star_ = is_star; }

  // Tile::set_letter()
  //
  // Modifies the data member `letter_`. Should never need use after
  // construction, save for in testing.
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