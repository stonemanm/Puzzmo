// -----------------------------------------------------------------------------
// File: path.h
// -----------------------------------------------------------------------------
//
// This header file defines paths. At its core, a path is a vector of Tile
// objects. Paths are used to construct words that can be cleared, but can be
// constructed with consecutive tiles that can never neighbor each other. To
// compensate, the class also contains logic to determine whether or not it is
// possible to legally create it in a game of Spelltower.

#ifndef path_h
#define path_h

#include <memory>
#include <vector>

#include "absl/strings/str_cat.h"
#include "tile.h"

namespace puzzmo::spelltower {

// spelltower::Path
//
// The `Path` class is used to assemble a word out of `Tile` objects for
// potential removal. At its core, it is a vector of shared_ptrs to tiles
// contained in a `Grid`.
//
// It is not necessarily possible to legally play the word in a path. To
// determine this is comparatively computationally expensive, so checking it is
// done in a standalone method.
class Path {
 public:
  //--------------
  // Constructors

  // The default constructor creates an empty path.
  Path() : simple_board_(9), star_count_(0) {}

  // If a vector of tiles is passed to the constructor, they will be added one
  // at a time.
  explicit Path(const std::vector<std::shared_ptr<Tile>> &tiles);

  //-----------
  // Accessors

  // Path::tiles()
  //
  // Provides access to the underlying vector of tiles.
  std::vector<std::shared_ptr<Tile>> tiles() const { return tiles_; }

  // operator[]
  //
  // The overloaded subscript operator provides access to the underlying vector.
  std::shared_ptr<Tile> &operator[](int i) { return tiles_[i]; }
  const std::shared_ptr<Tile> &operator[](int i) const { return tiles_[i]; }

  // Path::back()
  //
  // Provides access to the last entry in the underlying vector.
  std::shared_ptr<Tile> &back() { return tiles_.back(); }

  // Path::contains()
  //
  // Overloaded to provide two versions of the method. If a `Point` is passed
  // in, returns `true` if any of the tiles in the path have those coordinates.
  // If a `std::shared_ptr<Tile>` is passed in, returns true if that tile is
  // already in the path.
  bool contains(const Point &point) const;
  bool contains(const std::shared_ptr<Tile> &tile) const {
    return absl::c_contains(tiles_, tile);
  }

  // Path::empty()
  //
  // Returns `true` if tiles_ is empty.
  bool empty() const { return tiles_.empty(); }

  // Path::size()
  //
  // Returns the number of tiles in the path.
  int size() const { return tiles_.size(); }

  // Path::simple_board()
  //
  // Returns a 2D vector structured the same way a Grid is, but containing no
  // tiles other than those in the path and storing tiles by their indices in
  // `tiles_`. Used to examine the order of tiles in columns.
  std::vector<std::vector<int>> simple_board() const { return simple_board_; }

  // Path::min_possible_row()
  //
  // Stores the number of other tiles in this path that are below the tile at
  // `tiles_[i]` at index `i`.
  std::vector<int> min_possible_row() const { return min_possible_row_; }

  // Path::star_count()
  //
  // Returns the number of star tiles contained in this path.
  int star_count() const { return star_count_; }

  //---------
  // Strings

  // Path::TilesAsGrid()
  //
  // Returns a string formatted the same way that a `Grid` is formatted, but
  // with `kBlankTileLetter` in every space that is not a tile in the path.
  // Useful for visualizing the path.
  std::string TilesAsGrid() const;

  // Path::TilesAsString()
  //
  // Much in the same way that a string can be seen as an array of characters, a
  // path can be considered an array of tiles. This returns a string containing
  // each tile's character in order.
  std::string TilesAsString() const;

  //----------
  // Validity

  // Path::IsContinuous()
  //
  // Returns `true` if each pair of adjacent tiles are Moore neighbors.
  bool IsContinuous() const;

  // Path::IsPossible()
  //
  // Checks whether or not it is possible to make this path on a grid by
  // removing tiles not in this path.
  bool IsPossible() const;

  //----------
  // Mutators

  // Path::pop_back()
  //
  // Removes the last tile in the path and adjusts data accordingly.
  void pop_back();

  // Path::push_back()
  //
  // Adds this tile to the end of the path and adjusts data accordingly.
  void push_back(const std::shared_ptr<Tile> &tile);

  //---------
  // Members

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

  template <typename Sink>
  friend void AbslStringify(Sink &sink, const Path &path) {
    sink.Append(
        absl::StrCat("\"", path.TilesAsString(), "\"\n", path.TilesAsGrid()));
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