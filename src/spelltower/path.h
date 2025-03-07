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

#include "absl/status/status.h"
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

  // Path::lowest_legal_row()
  //
  // At index `i`, holds the lowest row to which that `tiles_[i]` can drop as
  // part of this path. This is determined by the number of path tiles beneath
  // it in `simple_board_`.
  std::vector<int> lowest_legal_row() const { return lowest_legal_row_; }

  // Path::adjusted_points()
  //
  // Returns the most recent entry in `adjusted_points_`, which corresponds to
  // the current path.
  std::vector<Point> adjusted_points() const;

  // Path::star_count()
  //
  // Returns the number of star tiles contained in this path.
  int star_count() const { return star_count_; }

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
  absl::Status IsPossible() const;

  //----------
  // Mutators

  // Path::pop_back()
  //
  // Removes the last tile in the path and adjusts data accordingly.
  void pop_back();

  // Path::push_back()
  //
  // Adds the tile or tiles to the end of the path and adjusts data accordingly.
  absl::Status push_back(const std::shared_ptr<Tile> &tile);
  absl::Status push_back(const std::vector<std::shared_ptr<Tile>> &tiles);

 private:
  // Path::AddNewestTileToSimpleBoard()
  //
  // A helper method for `push_back()` to update `simple_board_`. Undoes its
  // work and returns an error if this would create an interrupted column.
  absl::Status AddNewestTileToSimpleBoard();

  // Path::RemoveNewestTileFromSimpleBoard()
  //
  // A helper method for `pop_back()`.
  void RemoveNewestTileFromSimpleBoard();

  // Path::FindNewAdjustedPoints()
  //
  // Determines the least each tile in the path has to drop in order for the
  // path to become continuous, and saves the vector of updated points in
  // `adjusted_points`_.
  absl::Status FindNewAdjustedPoints();

  // Path::UpdatePoints()
  //
  // A helper method for `Path::IsPossible()`. Lowers the higher of two
  // points to be one row above the lower of the two, adjusting
  // others as needed. Returns false if it is not possible to do so.
  absl::Status MakePointsNeighbors(int idx_a, int idx_b,
                                   std::vector<Point> &points) const;

  //---------
  // Members

  std::vector<std::shared_ptr<Tile>> tiles_;
  std::vector<std::vector<int>> simple_board_;
  std::vector<int> lowest_legal_row_;
  std::vector<std::vector<Point>> adjusted_points_;
  int star_count_;

  //------------------
  // Abseil functions

  template <typename H>
  friend H AbslHashValue(H h, const Path &path) {
    return H::combine(std::move(h), path.tiles_, path.simple_board_,
                      path.lowest_legal_row_, path.star_count_);
  }

  template <typename Sink>
  friend void AbslStringify(Sink &sink, const Path &path) {
    for (const std::shared_ptr<Tile> &tile : path.tiles())
      absl::Format(&sink, "%v", *tile);
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