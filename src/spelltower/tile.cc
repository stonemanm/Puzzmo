#include "tile.h"

#include "absl/strings/str_cat.h"

namespace puzzmo::spelltower {

Tile::Tile(int row, int col, char letter)
    : coords_({.row = row, .col = col}),
      letter_(std::isalpha(letter) ? std::tolower(letter) : kBlankTileLetter),
      is_star_(std::isupper(letter)) {
  switch (letter_) {
    case 'a':
    case 'e':
    case 'i':
    case 'o':
    case 'u':
    case 's':
      value_ = 1;
      break;
    case 'l':
    case 'n':
    case 'r':
    case 't':
      value_ = 2;
      break;
    case 'd':
    case 'g':
      value_ = 3;
      break;
    case 'b':
    case 'c':
    case 'm':
    case 'p':
      value_ = 4;
      break;
    case 'f':
    case 'h':
    case 'v':
    case 'w':
    case 'y':
      value_ = 5;
      break;
    case 'k':
      value_ = 6;
      break;
    case 'j':
    case 'x':
      value_ = 9;
      break;
    case 'z':
      value_ = 11;
      break;
    case 'q':
      value_ = 12;
      break;
    default:
      value_ = 0;
  }
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