#include "path.h"

#include <algorithm>
#include <cmath>

#include "absl/strings/str_format.h"
#include "src/shared/point.h"

namespace puzzmo::spelltower {
namespace {

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

// A helper method for `Path::IsPossible()`. Lowers the higher of two
// consecutive points to be one row above the lower of the two, adjusting others
// as needed. Returns false if it is not possible to do so.
bool UpdatePoints(std::vector<Point> &points, int l,
                  const std::vector<int> &min_possible_row,
                  const std::vector<std::vector<int>> &simple_board) {
  int lo = (points[l].row < points[l + 1].row) ? l : l + 1;
  int hi = (points[l + 1].row < points[l].row) ? l : l + 1;
  if (points[lo].row + 1 < min_possible_row[hi]) return false;

  // We want to lower `hi` to one row above `lo`.
  int drop = points[hi].row - points[lo].row - 1;

  // Drop everything above `hi` the same amount. Recall that
  // `min_possible_row_[hi]` is the same as the index of `hi` in `simple_col`.
  std::vector<int> simple_col = simple_board[points[hi].col];
  for (int i = simple_col.size() - 1; i >= min_possible_row[hi]; --i) {
    int idx = simple_col[i];
    points[idx].row -= drop;
  }

  // Drop everything below 'hi' as little as possible. Note that we can safely
  // refer to `simple_col[i+1]`, as `hi` follows everything in this loop.
  for (int i = min_possible_row[hi] - 1; i >= 0; --i) {
    int idx = simple_col[i];
    // If this row doesn't need adjusting, none below it will.
    if (points[idx].row < points[simple_col[i + 1]].row) break;
    // If it does, then lower it as little as possible. This should not lower
    // anything to a row below 0, thanks to min_possible_row_.
    points[idx].row = points[simple_col[i + 1]].row - 1;
  }
  return true;
}

}  // namespace

Path::Path(const std::vector<std::shared_ptr<Tile>> &tiles)
    : simple_board_(9), star_count_(0) {
  for (const std::shared_ptr<Tile> &tile : tiles) push_back(tile);
}

bool Path::contains(const Point &p) const {
  for (const std::shared_ptr<Tile> &tile : tiles_) {
    if (tile == nullptr) continue;
    if (tile->coords() == p) return true;
  }
  return false;
}

void Path::pop_back() {
  const int idx = tiles_.size() - 1;
  std::shared_ptr<Tile> &tile = tiles_[idx];
  if (tile->is_star()) --star_count_;

  // Decrement `path_tiles_under_` for values above `tile`, then remove it from
  // `simple_col`.
  std::vector<int> &simple_col = simple_board_[tile->col()];
  int idx_in_simple_col = min_possible_row_[idx];
  for (int i = idx_in_simple_col + 1; i < simple_col.size(); ++i) {
    --min_possible_row_[simple_col[i]];
  }
  simple_col.erase(simple_col.begin() + idx_in_simple_col);

  min_possible_row_.pop_back();
  tiles_.pop_back();
}

void Path::push_back(const std::shared_ptr<Tile> &tile) {
  if (tile == nullptr) return;

  const int idx = tiles_.size();
  tiles_.push_back(tile);
  if (tile->is_star()) ++star_count_;

  // Set the number of tiles under `tile` to the number currently in
  // `simple_col`, then add it to `simple_col`.
  std::vector<int> &simple_col = simple_board_[tile->col()];
  min_possible_row_.push_back(simple_col.size());
  simple_col.push_back(idx);

  // Swap `idx` into its proper place in `simple_col`, updating
  // `path_tiles_under_` as we go.
  for (int t = simple_col.size() - 1; t > 0; --t) {
    if (tiles_[simple_col[t - 1]]->row() < tiles_[simple_col[t]]->row()) break;
    std::swap(min_possible_row_[simple_col[t - 1]],
              min_possible_row_[simple_col[t]]);
    std::swap(simple_col[t - 1], simple_col[t]);
  }
}

bool Path::IsContinuous() const {
  for (int i = 0; i < tiles_.size() - 1; ++i) {
    if (!tiles_[i]->coords().MooreNeighbors().contains(tiles_[i + 1]->coords()))
      return false;
  }
  return true;
}

bool Path::IsPossible() const {
  if (size() < 2) return true;
  // Make a vector of the coordinates of each point. We can modify this without
  // changing the path.
  std::vector<Point> points;
  for (const std::shared_ptr<Tile> &tile : tiles_)
    points.push_back(tile->coords());

  // Check for column gaps and interrupted columns (or "A-C-B columns")
  for (int i = 1; i < points.size(); ++i) {
    if (std::abs(points[i].col - points[i - 1].col) > 1) return false;
    if (points[i - 1].col == points[i].col &&
        std::abs(min_possible_row_[i] - min_possible_row_[i - 1]) > 1) {
      return false;
    }
  }

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
          if (!UpdatePoints(points, idx - 1, min_possible_row_, simple_board_))
            return false;
          break;  // Restarts the for loop.
        }
      }
      if (idx < points.size() - 1) {
        Point &next = points[idx + 1];
        if (!curr.MooreNeighbors().contains(next)) {
          if (!UpdatePoints(points, idx, min_possible_row_, simple_board_))
            return false;
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
  return true;
}

bool operator==(const Path &lhs, const Path &rhs) {
  return lhs.tiles() == rhs.tiles();
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