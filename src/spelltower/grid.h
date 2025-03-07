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
#include "absl/strings/str_join.h"
#include "path.h"
#include "src/shared/letter_count.h"
#include "tile.h"

namespace puzzmo::spelltower {

// spelltower::Grid
//
// The `Grid` class represents the state of the Spelltower board at any given
// point in play. At its most basic level, it is a two-dimensional matrix of
// `Tile` objects, which are stored as `std::shared_ptr<Tile>` for performance
// and convenience reasons.
//
// In addition, a `Grid` object holds data about the tiles that comprise it,
// which can be used when solving it. This includes a separate list of its star
// tiles, a `LetterCount` for each column, and a map from every letter on the
// board to all of the tiles that contain it.
class Grid {
 public:
  //-------------
  // Constructor

  // `Grid`s are created from a vector of strings. Each string represents a row,
  // with each character falling into each column. For each character, a `Tile`
  // object is created, with that character passed as its letter. If the
  // character is `kEmptySpaceLetter`, then a nullptr will be inserted instead.
  //
  // If `grid_strings` has more rows than `num_rows_`, only the bottommost rows
  // will be saved into the resulting object. If instead it has fewer rows, the
  // topmost rows will be left blank. Similarly, if any of the strings have
  // length above `num_cols_`, only the leftmost characters will be saved, and
  // if it has fewer columns, empty columns will be added at the right.
  //
  // If the input data is properly formatted, there should never be an empty
  // space underneath a tile. There are no built-in checks for this, and it will
  // cause unexpected issues.
  explicit Grid(const std::vector<std::string> &grid_strings);

  //-----------
  // Accessors

  // Grid::tiles()
  //
  // Returns a 2D vector of tiles. The inner vectors each contain a column, with
  // rows ordered from lowest row to highest. This will always contain the same
  // number of rows and columns; empty spaces will contain nullptr.
  std::vector<std::vector<std::shared_ptr<Tile>>> tiles() const {
    return tiles_;
  }

  // operator[]
  //
  // `Grid` has two separate overloaded subscript operators. If provided an int,
  // it treats it as the index of a column and returns that column. If provided
  // a `Point` instead, it returns the pointer at that row and column.
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

  // Grid::row()
  //
  // Constructs and returns a vector of the tiles in a row.
  std::vector<std::shared_ptr<Tile>> row(int row) const;

  // Grid::star_tiles()
  //
  // Returns a vector of pointers to the tiles in the grid that are star tiles.
  std::vector<std::shared_ptr<Tile>> star_tiles() const { return star_tiles_; }

  // Grid::letter_map()
  //
  // Returns a map from each letter to all the tiles with a given letter. This
  // map does not track blank tiles or empty spaces.
  absl::flat_hash_map<char, absl::flat_hash_set<std::shared_ptr<Tile>>>
  letter_map() const {
    return letter_map_;
  }

  // Grid::column_letter_counts()
  //
  // Returns a vector of LetterCounts, one for each column.
  std::vector<LetterCount> column_letter_counts() const {
    return column_letter_counts_;
  }

  //---------
  // Strings

  // Grid::VisualizePath()
  //
  // Returns a string that is formatted the same way that the grid would be
  // logged, except:
  // - Replace all letters that will be removed by the path, but are not part of
  //   it, with `kAffectedSpaceLetter`.
  // - Replace all letters neither in nor affected by the path with
  //   `kBlankTileLetter`.
  std::string VisualizePath(const Path &path) const;

  //---------
  // Scoring

  // Grid::ScorePath()
  //
  // Calculates the score from submitting the given path.
  //
  // Critically, ScorePath does not call `Path::IsPossible()` itself due to
  // computational expense. Ensure that `path` has been validated prior to
  // calling this method.
  int ScorePath(const Path &path) const;

  // Grid::AlmostThere()
  //
  // Returns `true` if no column in the grid has any tile in row 2 or above.
  bool AlmostThere() const;

  // Grid::FullClear()
  //
  // Returns `true` if all tiles have been cleared from the grid.
  bool FullClear() const;

  // Grid::ScoreBonuses()
  //
  // Returns the bonus points that this grid qualifies for in its current state.
  int ScoreBonuses() const { return 1000 * AlmostThere() + 1000 * FullClear(); }

  //---------------
  // Grid movement

  // Grid::AccessibleTilesFrom()
  //
  // Returns the Moore neighbors of `tile` in the grid. Empty spaces are not
  // included,
  absl::flat_hash_set<std::shared_ptr<Tile>> AccessibleTilesFrom(
      const std::shared_ptr<Tile> &tile) const;

  // Grid::IsPointInRange()
  //
  // Returns `true` if the point refers to a tile on the grid.
  bool IsPointInRange(const Point &p) const;

  // Grid::PossibleNextTilesForPath()
  //
  // Passes the Tile at `path.back()` to `Grid::AccessibleTilesFrom()`, then,
  // prior to returning, removes from the set all blank tiles and tiles already
  // in the path.
  absl::flat_hash_set<std::shared_ptr<Tile>> PossibleNextTilesForPath(
      const Path &path) const;

  // Grid::PointsAffectedBy()
  //
  // Returns the von Neumann neighbors of `tile->coords()` that have tiles on
  // them.
  absl::flat_hash_set<Point> PointsAffectedBy(
      const std::shared_ptr<Tile> &tile) const;

  // Grid::TilesAffectedBy()
  //
  // Returns `tile`'s von Neumann neighbors. Empty spaces are not included.
  absl::flat_hash_set<std::shared_ptr<Tile>> TilesAffectedBy(
      const std::shared_ptr<Tile> &tile) const;

  // Grid::TilesRemovedBy()
  //
  // Returns every `Tile` that will be removed if `path` is played. This
  // includes:
  //
  // - Every tile contained in `path`.
  // - For any rare tile in `path`, every other tile in that row.
  // - Every blank tile that is a von Neumann neighbor of a tile in `path`.
  // - If `path.size() >= 5`, every non-blank tile that is a von Neumann
  //   neighbor of a tile in `path`, as well.
  absl::flat_hash_set<std::shared_ptr<Tile>> TilesRemovedBy(
      const Path &path) const;

  //----------
  // Mutators

  // Grid::ClearPath()
  //
  // Removes all `TilesRemovedBy()` `path` from the grid, dropping tiles above
  // them into the empty spaces created.
  absl::Status ClearPath(const Path &path);

 private:
  // Grid::ClearTile()
  //
  // Removes `tile` from the grid, dropping any tiles above it into the empty
  // space created.
  absl::Status ClearTile(const std::shared_ptr<Tile> &tile);

  // Grid::AsCharMatrix()
  //
  // A helper method for `Grid::VisualizePath()` and `AbslStringify()`.
  std::vector<std::string> AsCharMatrix() const;

  // Grid::PointsRemovedBy()
  //
  // A helper method for `Grid::VisualizePath()` and `Grid::TilesRemovedBy()`.
  absl::flat_hash_set<Point> PointsRemovedBy(const Path &path) const;

  //---------
  // Members

  std::vector<std::vector<std::shared_ptr<Tile>>> tiles_;
  std::vector<std::shared_ptr<Tile>> star_tiles_;
  absl::flat_hash_map<char, absl::flat_hash_set<std::shared_ptr<Tile>>>
      letter_map_;
  std::vector<LetterCount> column_letter_counts_;

  static constexpr int kNumRows = 13;
  static constexpr int kNumCols = 9;
  static constexpr char kEmptySpaceLetter = ' ';
  static constexpr char kAffectedSpaceLetter = '+';

  //------------------
  // Abseil functions

  template <typename H>
  friend H AbslHashValue(H h, const Grid &grid) {
    return H::combine(std::move(h), grid.tiles_, grid.letter_map_,
                      grid.star_tiles_);
  }

  template <typename Sink>
  friend void AbslStringify(Sink &sink, const Grid &grid) {
    std::vector<std::string> board = grid.AsCharMatrix();
    std::reverse(board.begin(), board.end());
    sink.Append(absl::StrJoin(board, "\n"));
  }
};

bool operator==(const Grid &lhs, const Grid &rhs);
bool operator!=(const Grid &lhs, const Grid &rhs);

}  // namespace puzzmo::spelltower

#endif  // !grid_h