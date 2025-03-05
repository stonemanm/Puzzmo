#include "tile.h"

#include "absl/strings/str_cat.h"

namespace puzzmo::spelltower {

static constexpr int kLetterValues[] = {1, 4, 4, 3, 1, 5, 3, 5,  1,
                                        9, 6, 2, 4, 2, 1, 4, 12, 2,
                                        1, 2, 1, 5, 5, 9, 5, 11};

Tile::Tile(int row, int col, char letter)
    : coords_({.row = row, .col = col}),
      letter_(std::isalpha(letter) ? std::tolower(letter) : kBlankTileLetter),
      is_star_(std::isupper(letter)),
      value_(std::isalpha(letter_) ? kLetterValues[letter_ - 'a'] : 0) {}

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