#include "tile.h"

#include "absl/strings/str_cat.h"

namespace puzzmo::spelltower {

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