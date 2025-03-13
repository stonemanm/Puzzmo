#include "path.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "src/shared/point.h"

namespace puzzmo::spelltower {

constexpr absl::string_view kBlankTileError =
    "Cannot add a blank tile to a path.";
constexpr absl::string_view kColumnGapError =
    "The tile passed to push_back() is in column %d, which cannot be reached "
    "from column %d.";
constexpr absl::string_view kDuplicateTileError =
    "Path already contains tile %v, so it cannot be added again.";
constexpr absl::string_view kInterruptedColumnError =
    "Another path tile prevents any possible connection between this tile and "
    "the tile preceding it.";
constexpr absl::string_view kNullptrError = "Cannot add nullptr to the path.";
constexpr absl::string_view kPushBackError =
    "Path becomes impossible to create with this tile added.";
constexpr absl::string_view kTileNotOnGridError =
    "Tiles not on the grid cannot be added to the path.";

bool Path::contains(const Point &p) const {
  for (const std::shared_ptr<Tile> &tile : tiles_) {
    if (tile->coords() == p) return true;
  }
  return false;
}

bool Path::contains(const std::shared_ptr<Tile> &tile) const {
  return absl::c_contains(tiles_, tile);
}

std::vector<Point> Path::adjusted_points() const {
  if (adjusted_points_.empty()) return {};
  return adjusted_points_.back();
}

std::string Path::word() const {
  std::string word;
  for (const std::shared_ptr<Tile> &tile : tiles_)
    absl::StrAppend(&word, std::string(1, tile->letter()));
  return word;
}

bool Path::IsContinuous() const {
  for (int i = 0; i < tiles_.size() - 1; ++i) {
    if (!tiles_[i]->coords().MooreNeighbors().contains(tiles_[i + 1]->coords()))
      return false;
  }
  return true;
}

int Path::Delta() const {
  int delta = 0;
  for (int i = 0; i < tiles_.size(); ++i)
    delta += tiles_[i]->row() - adjusted_points()[i].row;
  return delta;
}

void Path::pop_back() {
  std::shared_ptr<Tile> tile = tiles_.back();
  adjusted_points_.pop_back();
  RemoveNewestTileFromSimpleBoard();
  if (tile->is_star()) --star_count_;
  tiles_.pop_back();
}

absl::Status Path::push_back(const std::shared_ptr<Tile> &tile) {
  if (tile == nullptr) return absl::InvalidArgumentError(kNullptrError);
  if (tile->is_blank()) return absl::InvalidArgumentError(kBlankTileError);
  if (!tile->is_on_grid())
    return absl::InvalidArgumentError(kTileNotOnGridError);
  if (std::any_of(tiles_.begin(), tiles_.end(),
                  [tile](const std::shared_ptr<Tile> &path_tile) {
                    return path_tile->coords() == tile->coords();
                  }))
    return absl::InvalidArgumentError(
        absl::StrFormat(kDuplicateTileError, *tile));
  if (!tiles_.empty() && std::abs(tile->col() - tiles_.back()->col()) > 1)
    return absl::OutOfRangeError(
        absl::StrFormat(kColumnGapError, tile->col(), tiles_.back()->col()));
  tiles_.push_back(tile);

  if (absl::Status s = AddNewestTileToSimpleBoard(); !s.ok()) {
    tiles_.pop_back();
    return s;
  }

  if (absl::Status s = AddNewestTileToAdjustedPoints(); !s.ok()) {
    RemoveNewestTileFromSimpleBoard();
    tiles_.pop_back();
    return s;
  }

  if (tile->is_star()) ++star_count_;
  return absl::OkStatus();
}

absl::Status Path::push_back(const std::vector<std::shared_ptr<Tile>> &tiles) {
  for (const std::shared_ptr<Tile> &tile : tiles)
    if (absl::Status s = push_back(tile); !s.ok()) return s;
  return absl::OkStatus();
}

absl::Status Path::AddNewestTileToSimpleBoard() {
  const std::shared_ptr<Tile> &tile = tiles_.back();
  std::vector<int> &simple_col = simple_board_[tile->col()];
  const int idx = size() - 1;

  // Insert `idx` at the correct place in `simple_col`.
  auto it = std::lower_bound(simple_col.begin(), simple_col.end(), idx,
                             [*this](int lhs, int rhs) {
                               return tiles_[lhs]->row() < tiles_[rhs]->row();
                             });
  int n = it - simple_col.begin();
  simple_col.insert(it, idx);
  lowest_legal_row_.push_back(n);

  // Update `lowest_legal_row_` for everything above `idx` in `simple_col`.
  for (int i = lowest_legal_row_[idx] + 1; i < simple_col.size(); ++i) {
    ++lowest_legal_row_[simple_col[i]];
  }
  if (simple_col.size() < 3) return absl::OkStatus();

  // If this creates an interrupted column, undo it and return an error.
  if (absl::c_contains(simple_col, idx - 1) &&
      std::abs(lowest_legal_row_[idx] - lowest_legal_row_[idx - 1]) > 1) {
    RemoveNewestTileFromSimpleBoard();
    return absl::OutOfRangeError(kInterruptedColumnError);
  }

  return absl::OkStatus();
}

void Path::RemoveNewestTileFromSimpleBoard() {
  std::vector<int> &simple_col = simple_board_[tiles_.back()->col()];
  const int idx = size() - 1;

  for (int i = lowest_legal_row_[idx] + 1; i < simple_col.size(); ++i) {
    --lowest_legal_row_[simple_col[i]];
  }
  simple_col.erase(simple_col.begin() + lowest_legal_row_[idx]);
  lowest_legal_row_.pop_back();
}

absl::Status Path::AddNewestTileToAdjustedPoints() {
  // If there is only one point, no adjustment is needed.
  if (adjusted_points_.empty()) {
    adjusted_points_.push_back({tiles_.back()->coords()});
    return absl::OkStatus();
  }

  absl::StatusOr<Point> p = SafePointToInsertLatestTile();
  if (!p.ok()) return p.status();

  std::vector<Point> points = adjusted_points_.back();
  points.push_back(*std::move(p));
  if (absl::Status s = AdjustPoints(points); !s.ok()) return s;

  adjusted_points_.push_back(points);
  return absl::OkStatus();
}

absl::StatusOr<Point> Path::SafePointToInsertLatestTile() {
  // It's possible that, for the path up to this point to be possible, the tile
  // we want to add must already have been removed. Even if not, we need to
  // ascertain how the number of rows that it must have dropped by this point.
  std::vector<Point> points = adjusted_points_.back();
  Point p = tiles_.back()->coords();
  const int p_idx = size() - 1;
  std::vector<int> simple_col = simple_board_[p.col];

  // (There's no need for a `ceiling_row`, as we can just update p.row)
  int floor_row = 0;

  // If `p` has another path tile beneath it, and that path tile has dropped `n`
  // tiles, `p` will have dropped at minimum `n` tiles as well.
  if (simple_col[0] != p_idx) {
    int idx_below = simple_col[lowest_legal_row_[p_idx] - 1];
    int n = tiles_[idx_below]->row() - points[idx_below].row;
    p.row -= n;
    // Additionally, it's impossible to place it at or below the row of that
    // tile.
    floor_row = points[idx_below].row + 1;
  }

  // If it has another path tile above it, that tile may have dropped far enough
  // to have pushed `latest_point` down as well.
  if (simple_col.back() != p_idx) {
    int idx_above = simple_col[lowest_legal_row_[p_idx] + 1];
    int n = tiles_[idx_above]->row() - points[idx_above].row;
    // We prioritize removing any non-path tiles sandwiched between `p` and the
    // point above it. After that, any further drop will also affect `p`.
    n -= std::max(tiles_[idx_above]->row() - tiles_[p_idx]->row() - 1, 0);
    if (n > 0) p.row -= n;
  }

  if (p.row < floor_row)
    return absl::OutOfRangeError("Tile has already been removed.");
  return p;
}

absl::Status Path::AdjustPoints(std::vector<Point> &points) {
  std::vector<int> sorted_indices(points.size());
  std::iota(sorted_indices.begin(), sorted_indices.end(), 0);

  bool is_aligned = false;
  while (!is_aligned) {
    // Start with the lowest point
    std::sort(sorted_indices.begin(), sorted_indices.end(),
              [points](int lhs, int rhs) {
                return points[lhs].row < points[rhs].row;
              });
    for (int i = 0; i < sorted_indices.size(); ++i) {
      int idx = sorted_indices[i];
      Point &p = points[idx];

      // If `p` has a predecessor and that predecessor is 2+ rows higher than
      // it, drop the predecessor into the neighborhood of `p`.
      if (idx > 0 && p.row + 1 < points[idx - 1].row) {
        if (absl::Status s = MakePointsNeighbors(idx - 1, idx, points); !s.ok())
            return s;
        break;  // Having adjusted a point, we restart the outer loop.
        }

      // If `p` has a successor and that successor is 2+ rows higher than it,
      // drop the successor into the neighborhood of `p`.
      if (idx < points.size() - 1 && p.row + 1 < points[idx + 1].row) {
        if (absl::Status s = MakePointsNeighbors(idx, idx + 1, points); !s.ok())
            return s;
        break;  // Having adjusted a point, we restart the outer loop.
      }
      // If we've made it here, this point (and all below it) can reach both of
      // their neighbors! If this is the final loop, set is_aligned = true so we
      // can break free
      if (i == sorted_indices.size() - 1) is_aligned = true;
    }
    if (is_aligned) break;
  }
  return absl::OkStatus();
}

absl::Status Path::MakePointsNeighbors(int idx_a, int idx_b,
                                       std::vector<Point> &points) const {
  // Determine which of the two points will move, and to where.
  int target_row = std::min(points[idx_a].row, points[idx_b].row) + 1;
  int idx_to_drop = points[idx_a].row > points[idx_b].row ? idx_a : idx_b;

  // If the lowest `idx_to_drop` can go is still out of reach of `fixed_idx`,
  // return an error.
  if (lowest_legal_row_[idx_to_drop] > target_row)
    return absl::OutOfRangeError(kPushBackError);

  std::vector<int> simple_col = simple_board_[points[idx_to_drop].col];
  int idx_in_simple_col = lowest_legal_row_[idx_to_drop];

  // It's possible that, in order to drop 'idx_to_drop', we have to drop
  // points beneath it as well.
  int ceiling_row = target_row;
  for (int i = idx_in_simple_col - 1; i >= 0; --i) {
    // If `idx` needs adjusting, lower it as little as possible.
    int idx = simple_col[i];
    if (ceiling_row > points[idx].row) break;
    points[idx].row = ceiling_row - 1;

    // Update `ceiling_row` to apply to the next point.
    ceiling_row = points[idx].row;
  }

  // When we drop `idx_to_drop`, everything above it drops by the same amount.
  int drop = points[idx_to_drop].row - target_row;
  for (int i = idx_in_simple_col; i < simple_col.size(); ++i)
    points[simple_col[i]].row -= drop;

  return absl::OkStatus();
}

bool operator==(const Path &lhs, const Path &rhs) {
  return lhs.tiles() == rhs.tiles() &&
         lhs.simple_board() == rhs.simple_board() &&
         lhs.lowest_legal_row() == rhs.lowest_legal_row() &&
         lhs.adjusted_points() == rhs.adjusted_points() &&
         lhs.star_count() == rhs.star_count();
}

bool operator!=(const Path &lhs, const Path &rhs) { return !(lhs == rhs); }

bool operator<(const Path &lhs, const Path &rhs) {
  int size = std::min(lhs.size(), rhs.size());
  for (int i = 0; i < size; ++i) {
    std::string l = absl::StrFormat("%v", *lhs[i]);
    std::string r = absl::StrFormat("%v", *rhs[i]);
    if (l != r) return l < r;
  }
  return lhs.size() < rhs.size();
}

bool operator<=(const Path &lhs, const Path &rhs) {
  return lhs < rhs || lhs == rhs;
}

bool operator>(const Path &lhs, const Path &rhs) { return rhs < lhs; }

bool operator>=(const Path &lhs, const Path &rhs) { return rhs <= lhs; }

}  // namespace puzzmo::spelltower