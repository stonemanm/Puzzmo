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
namespace {

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

// A helper method for `Path::IsPossible()`. Returns the tile indices in
// `path`, sorted so as to iterate from the lowest to the highest row.
std::vector<int> IndicesByRow(const std::vector<Point> &points) {
  std::vector<int> indices(points.size());
  std::iota(indices.begin(), indices.end(), 0);
  std::sort(indices.begin(), indices.end(), [points](int lhs, int rhs) {
    return points[lhs].row < points[rhs].row;
  });
  return indices;
}

}  // namespace

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

  if (absl::Status s = FindNewAdjustedPoints(); !s.ok()) {
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

absl::Status Path::FindNewAdjustedPoints() {
  const int new_idx = size() - 1;
  Point new_p = tiles_.back()->coords();

  // If there is only one point, no adjustment is needed.
  if (adjusted_points_.empty()) {
    adjusted_points_.push_back({new_p});
    return absl::OkStatus();
  }
  std::vector<Point> points = adjusted_points_.back();

  // Now we have to determine if `new_p` must have already been removed. If it
  // has, then obviously it cannot be added to the path. If not, then we need to
  // determine its highest possible row as of the most recent entry in
  // `adjusted_points_`.
  std::vector<int> simple_col = simple_board_[new_p.col];
  bool has_path_point_above = (simple_col.back() != new_idx);
  bool has_path_point_below = (simple_col[0] != new_idx);
  int floor_row = 0;

  // If it has something below it, then it will have dropped, at minimum, as
  // many rows as the point beneath it has.
  if (has_path_point_below) {
    int idx_below = simple_col[lowest_legal_row_[new_idx] - 1];
    int rows_dropped_by_point_below =
        tiles_[idx_below]->row() - points[idx_below].row;
    new_p.row -= rows_dropped_by_point_below;
    floor_row = points[idx_below].row + 1;
  }

  // If it has something above it, then the movement of that point may have
  // dropped `new_p` as well.
  if (has_path_point_above) {
    int idx_above = simple_col[lowest_legal_row_[new_idx] + 1];
    // `tiles_between_them` is the amount that above can have dropped without
    // affecting `new_p` at all.
    int tiles_between_them = tiles_[idx_above]->row() - tiles_[new_idx]->row();
    if (tiles_between_them > 0) --tiles_between_them;
    int rows_dropped_by_point_above =
        tiles_[idx_above]->row() - points[idx_above].row;
    int rows_new_p_must_drop = rows_dropped_by_point_above - tiles_between_them;
    if (rows_new_p_must_drop > 0) new_p.row -= rows_new_p_must_drop;
  }

  if (new_p.row < floor_row)
    return absl::OutOfRangeError("Tile has already been removed.");
  points.push_back(new_p);

  // LOG(INFO) << "After inserting new_p (size " << size()
  //           << "): " << absl::StrJoin(points, ", ");

  // Now that we've placed `new_p`, we need to make the path continuous by
  // dropping points as needed.
  bool is_aligned = false;
  while (!is_aligned) {
    std::vector<int> sorted_indices = IndicesByRow(points);

    // Start with the lowest point
    for (int i = 0; i < sorted_indices.size(); ++i) {
      int idx = sorted_indices[i];
      Point &curr = points[idx];

      if (idx > 0) {
        Point &prev = points[idx - 1];
        if (!curr.MooreNeighbors().contains(prev)) {
          if (absl::Status s = MakePointsNeighbors(idx - 1, idx, points);
              !s.ok())
            return s;
          break;  // Restarts the for loop.
        }
      }

      if (idx < points.size() - 1) {
        Point &next = points[idx + 1];
        if (!curr.MooreNeighbors().contains(next)) {
          if (absl::Status s = MakePointsNeighbors(idx, idx + 1, points);
              !s.ok())
            return s;
          break;  // Restarts the for loop.
        }
      }
      // If we've made it here, this point (and all below it) can reach both of
      // their neighbors! If this is the final loop, set is_aligned = true so we
      // can break free
      if (i == sorted_indices.size() - 1) is_aligned = true;
    }
    if (is_aligned) break;
  }
  // LOG(INFO) << "Adjusted points after adding point #" << size() << ":";
  // for (int i = 0; i < size() - 1; ++i) {
  //   LOG(INFO) << tiles_[i]->letter_on_board() << " " << tiles_[i]->coords()
  //             << " -> " << points[i];
  // }
  // LOG(INFO) << tiles_[size() - 1]->letter_on_board() << " "
  //           << tiles_[size() - 1]->coords() << " -> " << points[size() - 1];
  adjusted_points_.push_back(points);
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