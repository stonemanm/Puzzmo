#include "tile.h"

#include "absl/strings/str_cat.h"

namespace puzzmo::spelltower {

Tile::Tile(int row, int col, char letter, bool is_star)
    : coords_({.row = row, .col = col}), letter_(letter), is_star_(is_star) {
  if (!std::isalpha(letter)) letter_ = kBlankTileLetter;
}

absl::Status Tile::Drop(int rows) {
  if (rows > coords_.row)
    return absl::OutOfRangeError(absl::StrCat("Tile ", *this, " at ", coords_,
                                              " is not high enough to drop it ",
                                              rows, " rows."));
  coords_.row -= rows;
  return absl::OkStatus();
}

bool operator==(const Tile &lhs, const Tile &rhs) {
  return lhs.coords() == rhs.coords() && lhs.letter() == rhs.letter() &&
         lhs.is_star() == rhs.is_star();
}

bool operator!=(const Tile &lhs, const Tile &rhs) { return !(lhs == rhs); }

}  // namespace puzzmo::spelltower