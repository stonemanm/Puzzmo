// -----------------------------------------------------------------------------
// File: path.h
// -----------------------------------------------------------------------------
//
// This header file defines paths. At its core, a path is a vector of Tile
// objects. Paths are used to construct words that can be cleared, but can be
// constructed with consecutive tiles that can never neighbor each other. It is
// impossible to add a new tile to a path if there is no way of shifting tiles
// to make the path continuous.
//
// Note that `Path` does not own the tiles that comprise it. It is possible to
// move tiles in a path such that the path is no longer possible. There are
// currently no safeguards around this, so it is recommended for the time being
// that a path not outlast the modification of the grid it was based upon.

#ifndef PUZZMO_SPELLTOWER_PATH_H_
#define PUZZMO_SPELLTOWER_PATH_H_

#include <memory>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "tile.h"

namespace puzzmo::spelltower {

// spelltower::Path
//
// The `Path` class is used to assemble a word out of `Tile` objects for
// potential removal. At its core, it is a vector of shared_ptrs to tiles
// contained in a `Grid`.
//
// A path is "continuous" if, for every tile in the path, both its predecessor
// and its successor (if any) are Moore neighbors of that tile--that is to say,
// one of the up-to-eight tiles surrounding it. Paths must be continuous to be
// played. Tiles can be added to a `Path` object even if doing so will result in
// a non-continuous path, so long as it is possible for the path to become
// continuous by means of lowering tiles in the path.
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
  bool contains(const std::shared_ptr<Tile> &tile) const;

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
  // Returns a vector that, at index `i`, holds the lowest row to which
  // `tiles_[i]` can drop as part of this path. This is determined by the number
  // of path tiles beneath it in `simple_board_`.
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

  // Path::word()
  //
  // Returns the word spelled out by the tiles in the path, cast to lowercase.
  std::string word() const;

  //----------
  // Validity

  // Path::IsContinuous()
  //
  // Returns `true` if each pair of adjacent tiles are Moore neighbors.
  bool IsContinuous() const;

  // Path::IsOnGrid()
  //
  // Returns `true` if every tile in the path is on the grid.
  bool IsOnGrid() const;

  // Path::IsStillPossible()
  //
  // Returns `true` if the coordinates of the path have not been adjusted so as
  // to make the path impossible.
  bool IsStillPossible() const;

  // Path::TilesToDrop()
  //
  // Returns a vector constructed such that the index of each path tile contains
  // the number of rows it needs to drop in order to be continuous. If this is
  // impossible, returns an empty vector.
  std::vector<int> TilesToDrop() const;

  // Path::Delta()
  //
  // Returns the sum, for each tile in the path, of the difference between
  // its coords and its `adjusted_coords_`. A continuous path will have a
  // delta of zero.
  int Delta() const;

  // Path::MultiplierWhenScored()
  //
  // Returns the multiplier from the path, a function of its size and number of
  // star tiles.
  int MultiplierWhenScored() const { return size() * (1 + star_count_); }

  //----------
  // Mutators

  // Path::pop_back()
  //
  // Removes the last tile in the path and adjusts data accordingly.
  void pop_back();

  // Path::push_back()
  //
  // Attempts to add the tile or tiles to the path. Cannot accept `nullptr`,
  // blank tiles, or duplicate tiles, returning an error. Likewise, if adding a
  // tile will render it impossible for the path to ever become continuous, an
  // error will be returned.
  //
  // Should `push_back()` return something other than `absl::OkStatus()`, the
  // path object will be left in a valid state. If a vector of tiles has been
  // provided, only the tiles preceding that which caused the error will have
  // been added.
  absl::Status push_back(const std::shared_ptr<Tile> &tile);
  absl::Status push_back(const std::vector<std::shared_ptr<Tile>> &tiles);

 private:
  // Path::AddNewestTileToSimpleBoard()
  //
  // A helper method for `Path::push_back()` to update `simple_board_`. Undoes
  // its work and returns an error if this would create an interrupted column.
  absl::Status AddNewestTileToSimpleBoard();

  // Path::RemoveNewestTileFromSimpleBoard()
  //
  // A helper method for `pop_back()`.
  void RemoveNewestTileFromSimpleBoard();

  // Path::AddNewestTileToAdjustedPoints()
  //
  // Determines the least each tile in the path has to drop in order for the
  // path to become continuous, and saves the vector of updated points in
  // `adjusted_points`_.
  absl::Status AddNewestTileToAdjustedPoints();

  // Path::SafePlaceToInsertLatestTile()
  //
  // Returns the highest point at which the latest tile can be inserted into
  // `adjusted_points_`.
  absl::StatusOr<Point> SafePointToInsertLatestTile() const;

  // Path::AdjustPoints()
  //
  // Lowers the points as little as possible in order to make them continuous.
  // If this is impossible, returns an error.
  absl::Status AdjustPoints(std::vector<Point> &points) const;

  // Path::UpdatePoints()
  //
  // A helper method for `Path::push_back()`. Lowers the higher of two
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
    sink.Append(absl::StrJoin(
        path.tiles(), ", ",
        [](std::string *out, const std::shared_ptr<Tile> &tile) {
          return absl::StrAppend(out, absl::StrFormat("%v", *tile));
        }));
  }
};

bool operator==(const Path &lhs, const Path &rhs);
bool operator!=(const Path &lhs, const Path &rhs);
bool operator<(const Path &lhs, const Path &rhs);
bool operator<=(const Path &lhs, const Path &rhs);
bool operator>(const Path &lhs, const Path &rhs);
bool operator>=(const Path &lhs, const Path &rhs);

}  // namespace puzzmo::spelltower

#endif