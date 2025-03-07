#include "path.h"

#include <algorithm>
#include <cmath>
#include <string>

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
constexpr absl::string_view kInterruptedColumnError =
    "Another path tile prevents any possible connection between this tile and "
    "the tile preceding it.";
constexpr absl::string_view kNullptrError = "Cannot add nullptr to the path.";
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

void Path::pop_back() {
  const int idx = tiles_.size() - 1;
  std::shared_ptr<Tile> &tile = tiles_[idx];
  if (tile->is_star()) --star_count_;
  RemoveNewestTileFromSimpleBoard();
  tiles_.pop_back();
}

absl::Status Path::push_back(const std::shared_ptr<Tile> &tile) {
  if (tile == nullptr) return absl::InvalidArgumentError(kNullptrError);
  if (tile->is_blank()) return absl::InvalidArgumentError(kBlankTileError);
  if (!tiles_.empty() && std::abs(tile->col() - tiles_.back()->col()) > 1)
    return absl::OutOfRangeError(
        absl::StrFormat(kColumnGapError, tile->col(), tiles_.back()->col()));

  tiles_.push_back(tile);
  if (absl::Status s = AddNewestTileToSimpleBoard(); !s.ok()) {
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

bool Path::IsContinuous() const {
  for (int i = 0; i < tiles_.size() - 1; ++i) {
    if (!tiles_[i]->coords().MooreNeighbors().contains(tiles_[i + 1]->coords()))
      return false;
  }
  return true;
}

absl::Status Path::IsPossible() const {
  if (size() < 2) return absl::OkStatus();
  // Make a vector of the coordinates of each point. We can modify this without
  // changing the path.
  std::vector<Point> points;
  for (const std::shared_ptr<Tile> &tile : tiles_)
    points.push_back(tile->coords());

  // Now check for row gaps, dropping the points when necessary.
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

absl::Status Path::MakePointsNeighbors(int idx_a, int idx_b,
                                       std::vector<Point> &points) const {
  // Determine which of the two points is fixed and which will move.
  int fixed_idx = points[idx_a].row < points[idx_b].row ? idx_a : idx_b;
  int loose_idx = points[idx_b].row < points[idx_a].row ? idx_a : idx_b;

  // If the lowest `loose_idx` can go is still out of reach of `fixed_idx`,
  // return an error.
  int target_row = points[fixed_idx].row + 1;
  if (lowest_legal_row_[loose_idx] > target_row)
    return absl::InvalidArgumentError(
        "Unable to make the path possible by dropping points.");

  std::vector<int> simple_col = simple_board_[points[loose_idx].col];
  int idx_of_loose_idx_in_simple_col = lowest_legal_row_[loose_idx];

  // It's possible that, in order to drop 'loose_idx', we have to drop
  // points beneath it as well.
  int ceiling_row = target_row;
  for (int i = idx_of_loose_idx_in_simple_col - 1; i >= 0; --i) {
    // If `idx` needs adjusting, lower it as little as possible.
    int idx = simple_col[i];
    if (ceiling_row > points[idx].row) break;
    points[idx].row = ceiling_row - 1;

    // Update `ceiling_row` to apply to the next point.
    ceiling_row = points[idx].row;
  }

  // When we drop `loose_idx`, everything above it drops by the same amount.
  int drop = points[loose_idx].row - target_row;
  for (int i = idx_of_loose_idx_in_simple_col; i < simple_col.size(); ++i)
    points[simple_col[i]].row -= drop;

  return absl::OkStatus();
}

bool operator==(const Path &lhs, const Path &rhs) {
  return lhs.tiles() == rhs.tiles() &&
         lhs.simple_board() == rhs.simple_board() &&
         lhs.lowest_legal_row() == rhs.lowest_legal_row() &&
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