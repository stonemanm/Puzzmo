// -----------------------------------------------------------------------------
// File: grid.h
// -----------------------------------------------------------------------------
//
// This header file defines grids. A grid is the board state of Spelltower at
// any given moment. It consists of a 9x13 layout of tiles, some of which are
// star tiles.

#ifndef grid_h
#define grid_h

#include <memory>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/status/status.h"
#include "path.h"
#include "src/shared/letter_count.h"
#include "tile.h"

namespace puzzmo::spelltower {

constexpr char kEmptySpaceLetter = ' ';

class Grid {
 public:
  explicit Grid(const std::vector<std::string> &grid);

  //-----------
  // Accessors

  // Overloaded subscript operators. If given a point, returns the corresponding
  // Tile. If given an int, returns a column vector.
  std::shared_ptr<Tile> &operator[](Point p) { return tiles_[p.col][p.row]; }
  const std::shared_ptr<Tile> &operator[](Point p) const {
    return tiles_[p.col][p.row];
  }
  std::vector<std::shared_ptr<Tile>> &operator[](int col) {
    return tiles_[col];
  }
  const std::vector<std::shared_ptr<Tile>> &operator[](int col) const {
    return tiles_[col];
  }

  // Constructs and returns a vector of the tiles in a row.
  std::vector<std::shared_ptr<Tile>> row(int row) const;

  // Returns a 2D vector of tiles. The inner vectors each contain a column, from
  // lowest row to highest. This will always contain the same number of rows and
  // columns; empty spaces will contain nullptr.
  std::vector<std::vector<std::shared_ptr<Tile>>> tiles() const {
    return tiles_;
  }

  // Returns the proper subset of tiles that are starred.
  std::vector<std::shared_ptr<Tile>> star_tiles() const { return star_tiles_; }

  // Returns a map to all the tiles with a given letter.
  absl::flat_hash_map<char, absl::flat_hash_set<std::shared_ptr<Tile>>>
  letter_map() const {
    return letter_map_;
  }

  // Returns a vector of LetterCounts, one for each column.
  std::vector<LetterCount> column_letter_counts() const {
    return column_letter_counts_;
  }

  //---------
  // Bonuses

  // True iff no column has more than two tiles left in it.
  bool AlmostThere() const;

  // True iff all tiles have been cleared from the grid.
  bool FullClear() const;

  //---------------
  // Grid movement

  // Returns `tile`'s Moore neighbors.
  absl::flat_hash_set<std::shared_ptr<Tile>> AccessibleTilesFrom(
      const std::shared_ptr<Tile> &tile) const;

  // Returns true iff the point refers to a tile on the grid.
  bool IsPointInRange(const Point &p) const;

  // Calls `Path::AccessibleTilesFrom()` on `path.back()`, removing any blank
  // tiles or tiles already in the path.
  absl::flat_hash_set<std::shared_ptr<Tile>> PossibleNextTilesForPath(
      const Path &path) const;

  // Returns `tile`'s von Neumann neighbors.
  absl::flat_hash_set<std::shared_ptr<Tile>> TilesAffectedBy(
      const std::shared_ptr<Tile> &tile) const;

  // Returns all tiles that will be removed if `path` is played, including
  // `Grid::TilesAffectedBy` for every point in  `path` if it has 5 or more
  // tiles, and including anything in the same row as a 'j', 'q', 'x', or 'z'
  // that is in the path.
  absl::flat_hash_set<std::shared_ptr<Tile>> TilesRemovedBy(
      const Path &path) const;

  //----------
  // Mutators

  // Removes all tiles removed by the path from the grid.
  absl::Status ClearPath(const Path &path);

  // Removes the tile from the grid.
  absl::Status ClearTile(const std::shared_ptr<Tile> &tile);

  //---------
  // Scoring

  // Calculates the score from submitting the given path. Critically, ScorePath
  // does not call `Path::IsPossible()` itself, as it is comparatively intensive
  // to compute.
  int ScorePath(const Path &path) const;

  // Returns the points from the bonuses this grid has qualified for.
  int ScoreBonuses() const { return 1000 * AlmostThere() + 1000 * FullClear(); }

 private:
  std::vector<std::vector<std::shared_ptr<Tile>>> tiles_;
  std::vector<std::shared_ptr<Tile>> star_tiles_;
  absl::flat_hash_map<char, absl::flat_hash_set<std::shared_ptr<Tile>>>
      letter_map_;
  std::vector<LetterCount> column_letter_counts_;

  static constexpr int num_rows_ = 13;
  static constexpr int num_cols_ = 9;

  //------------------
  // Abseil functions

  template <typename H>
  friend H AbslHashValue(H h, const Grid &grid) {
    return H::combine(std::move(h), grid.tiles_, grid.letter_map_,
                      grid.star_tiles_);
  }

  template <typename Sink>
  friend void AbslStringify(Sink &sink, const Grid &grid) {
    for (int r = grid.num_rows_ - 1; r >= 0; --r) {
      std::vector<std::shared_ptr<Tile>> row = grid.row(r);
      if (r < grid.num_rows_ - 1) sink.Append("\n");
      for (const std::shared_ptr<Tile> &tile : row) {
        if (tile == nullptr)
          sink.Append(std::string(1, kEmptySpaceLetter));
        else
          absl::Format(&sink, "%v", *tile);
      }
    }
  }
};

bool operator==(const Grid &lhs, const Grid &rhs);
bool operator!=(const Grid &lhs, const Grid &rhs);

}  // namespace puzzmo::spelltower

#endif  // !grid_h