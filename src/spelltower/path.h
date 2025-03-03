// -----------------------------------------------------------------------------
// File: path.h
// -----------------------------------------------------------------------------
//
// This header file defines paths. At its core, a path is a vector of tiles. The
// class also contains logic to determine whether or not it is possible to
// legally create it in a game of Spelltower.

#ifndef path_h
#define path_h

#include <memory>
#include <vector>

#include "absl/strings/str_join.h"
#include "tile.h"

namespace puzzmo::spelltower {

class Path {
 public:
  Path() : simple_board_(9), star_count_(0) {}
  explicit Path(const std::vector<std::shared_ptr<Tile>> &tiles);

  //-----------
  // Accessors

  // Overloaded subscript operators for tile access.
  std::shared_ptr<Tile> &operator[](int i) { return tiles_[i]; }
  const std::shared_ptr<Tile> &operator[](int i) const { return tiles_[i]; }
  std::shared_ptr<Tile> &back() { return tiles_.back(); }
  bool contains(const std::shared_ptr<Tile> &tile) const {
    return absl::c_contains(tiles_, tile);
  }
  bool contains(const Point &point) const;
  bool empty() const { return tiles_.empty(); }
  int size() const { return tiles_.size(); }

  // Provides access to the underlying vector of tiles.
  std::vector<std::shared_ptr<Tile>> tiles() const { return tiles_; }

  std::string TilesAsString() const;

  // Returns a 2D vector structured the same way a Grid is, but containing no
  // tiles other than those in the path. Used to examine the order of tiles in
  // columns. Tiles are stored by their indices in `tiles_`.
  std::vector<std::vector<int>> simple_board() const { return simple_board_; }

  // Stores the number of other tiles in this path that are below the tile at
  // `tiles_[i]` at index `i`.
  std::vector<int> min_possible_row() const { return min_possible_row_; }

  // Returns the number of star tiles contained in this path.
  int star_count() const { return star_count_; }

  //----------
  // Mutators

  // Removes the last tile in the path and adjusts data accordingly.
  void pop_back();

  // Adds this tile to the end of the path and adjusts data accordingly.
  void push_back(const std::shared_ptr<Tile> &tile);

  //----------
  // Validity

  // Checks whether or not it is possible to make this path on a grid by
  // removing tiles not in this path.
  bool IsPossible() const;

 private:
  std::vector<std::shared_ptr<Tile>> tiles_;
  std::vector<std::vector<int>> simple_board_;
  std::vector<int> min_possible_row_;
  int star_count_;

  //------------------
  // Abseil functions

  template <typename H>
  friend H AbslHashValue(H h, const Path &path) {
    return H::combine(std::move(h), path.tiles_, path.simple_board_,
                      path.min_possible_row_, path.star_count_);
  }

  // TODO: swap in TilesAsString here, and make TilesOnGrid or something?
  template <typename Sink>
  friend void AbslStringify(Sink &sink, const Path &path) {
    std::vector<std::string> board(13, std::string(9, kBlankTileLetter));
    for (const std::shared_ptr<Tile> &tile : path.tiles_) {
      if (tile == nullptr) continue;
      board[12 - tile->row()][tile->col()] =
          tile->is_star() ? std::toupper(tile->letter()) : tile->letter();
    }

    sink.Append(absl::StrCat("\"", path.TilesAsString(), "\"\n",
                             absl::StrJoin(board, "\n")));
  }
};

bool operator==(const Path &lhs, const Path &rhs);
bool operator!=(const Path &lhs, const Path &rhs);
bool operator<(const Path &lhs, const Path &rhs);
bool operator<=(const Path &lhs, const Path &rhs);
bool operator>(const Path &lhs, const Path &rhs);
bool operator>=(const Path &lhs, const Path &rhs);

}  // namespace puzzmo::spelltower

#endif  // !path_h