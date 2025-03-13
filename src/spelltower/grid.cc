#include "grid.h"

#include <algorithm>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"

namespace puzzmo::spelltower {

Grid::Grid(const std::vector<std::string>& grid_strings)
    : tiles_(kNumCols, std::vector<std::shared_ptr<Tile>>(kNumRows, nullptr)),
      column_letter_counts_(kNumCols) {
  const int max_row = std::min((int)grid_strings.size(), kNumRows);
  for (int r = 0; r < max_row; ++r) {
    int idx = grid_strings.size() - 1 - r;  // Start at the last string.

    const int max_col = std::min((int)grid_strings[idx].size(), kNumCols);
    for (int c = 0; c < max_col; ++c) {
      const char l = grid_strings[idx][c];
      if (l == kEmptySpaceLetter) continue;

      std::shared_ptr<Tile> tile = std::make_shared<Tile>(r, c, l);
      tiles_[c][r] = tile;
      if (tile->is_star()) star_tiles_.push_back(tile);
      if (tile->is_blank()) continue;

      letter_map_[tile->letter()].insert(tile);
      (void)column_letter_counts_[c].AddLetter(tile->letter());
    }
  }
}

std::vector<std::shared_ptr<Tile>> Grid::row(int row) const {
  std::vector<std::shared_ptr<Tile>> v;
  for (int col = 0; col < kNumCols; ++col) {
    Point p = {row, col};
    v.push_back(IsPointInRange(p) ? tiles_[col][row] : nullptr);
  }
  return v;
}

int Grid::ScorePath(const Path& path) const {
  std::vector<std::shared_ptr<Tile>> affected_tiles = TilesRemovedBy(path);

  // Sum the values of all affected points, multiply by path.size(), and
  // multiply again by the number of stars used (plus one).
  int score = 0;
  for (const std::shared_ptr<Tile>& tile : affected_tiles) {
    if (tile == nullptr || tile->is_blank()) continue;
    score += tile->value();
  }
  score *= path.size();
  return score *= (1 + path.star_count());
}

bool Grid::AlmostThere() const {
  return std::all_of(
      tiles_.begin(), tiles_.end(),
      [](const std::vector<std::shared_ptr<Tile>>& column) {
        return absl::c_count_if(column, [](const std::shared_ptr<Tile>& tile) {
                 return tile != nullptr;
               }) <= 2;
      });
}

bool Grid::FullClear() const {
  return std::all_of(
      tiles_.begin(), tiles_.end(),
      [](const std::vector<std::shared_ptr<Tile>>& column) {
        return std::all_of(
            column.begin(), column.end(),
            [](const std::shared_ptr<Tile>& tile) { return tile == nullptr; });
      });
}

absl::flat_hash_set<std::shared_ptr<Tile>> Grid::AccessibleTilesFrom(
    const std::shared_ptr<Tile>& tile) const {
  if (tile == nullptr) return {};

  absl::flat_hash_set<Point> moore_neighbors = tile->coords().MooreNeighbors();
  absl::erase_if(moore_neighbors,
                 [*this](const Point& p) { return !IsPointInRange(p); });

  absl::flat_hash_set<std::shared_ptr<Tile>> accessible_tiles;
  for (const Point& p : moore_neighbors) {
    std::shared_ptr<Tile> tile = tiles_[p.col][p.row];
    if (tile == nullptr) continue;
    accessible_tiles.insert(tile);
  }
  return accessible_tiles;
}

bool Grid::IsPointInRange(const Point& p) const {
  if (p.row < 0 || p.row >= kNumRows || p.col < 0 || p.col >= kNumCols)
    return false;
  return tiles_[p.col][p.row] != nullptr;
}

absl::flat_hash_set<std::shared_ptr<Tile>> Grid::PossibleNextTilesForPath(
    const Path& path) const {
  absl::flat_hash_set<std::shared_ptr<Tile>> accessible_tiles =
      AccessibleTilesFrom(path.tiles().back());
  absl::erase_if(accessible_tiles, [path](const std::shared_ptr<Tile>& tile) {
    return tile->is_blank() || path.contains(tile->coords());
  });
  return accessible_tiles;
}

absl::flat_hash_set<Point> Grid::PointsAffectedBy(
    const std::shared_ptr<Tile>& tile) const {
  if (tile == nullptr) return {};

  absl::flat_hash_set<Point> affected_points =
      tile->coords().VonNeumannNeighbors();
  absl::erase_if(affected_points,
                 [*this](const Point& p) { return !IsPointInRange(p); });

  return affected_points;
}

std::vector<Point> Grid::PointsRemovedBy(const Path& path) const {
  absl::flat_hash_set<Point> affected_points;
  for (const std::shared_ptr<Tile>& tile : path.tiles()) {
    // Because `tile` is coming from `path`, we know it isn't `nullptr`.
    affected_points.insert(tile->coords());

    // If `tile` is rare, we need to include everything in the row.
    if (tile->is_rare()) {
      for (int c = 0; c < kNumCols; ++c)
        if (Point p = {tile->row(), c}; IsPointInRange(p))
          affected_points.insert(p);
    }

    // We want to include any blank tiles that are von Neumann neighbors
    // regardless, but if `path` has 5 or more tiles, we keep the letters as
    // well.
    absl::flat_hash_set<Point> points_affected_by = PointsAffectedBy(tile);
    for (const Point& p : points_affected_by) {
      if (path.size() < 5 && !tiles_[p.col][p.row]->is_blank()) continue;
      affected_points.insert(p);
    }
  }
  std::vector<Point> vec(affected_points.begin(), affected_points.end());
  std::sort(vec.begin(), vec.end(), [](const Point& lhs, const Point& rhs) {
    return lhs.col != rhs.col ? lhs.col < rhs.col : lhs.row > rhs.row;
  });
  return vec;
}

std::vector<std::shared_ptr<Tile>> Grid::TilesRemovedBy(
    const Path& path) const {
  std::vector<std::shared_ptr<Tile>> removed_tiles;

  std::vector<Point> affected_points = PointsRemovedBy(path);
  for (const Point& p : affected_points) {
    // This should never be `nullptr` due to checks in `PointsRemovedBy()`.
    std::shared_ptr<Tile> tile = tiles_[p.col][p.row];
    removed_tiles.push_back(tile);
  }
  return removed_tiles;
}

std::vector<std::string> Grid::AsCharMatrix() const {
  std::vector<std::string> v;
  for (int r = 0; r < kNumRows; ++r) {
    std::vector<std::shared_ptr<Tile>> grid_row = row(r);
    while (!grid_row.empty() && grid_row.back() == nullptr) grid_row.pop_back();
    if (grid_row.empty())
      break;  // If a row is completely empty, those above it are too.

    std::string s;
    for (const std::shared_ptr<Tile>& tile : grid_row) {
      if (tile == nullptr)
        absl::StrAppend(&s, std::string(1, kEmptySpaceLetter));
      else
        absl::StrAppend(&s, tile->letter_on_board());
    }
    v.push_back(s);
  }
  return v;
}

std::string Grid::NStarRegex(int n) const {
  if (n < 2 || star_tiles_.size() < n) return "";

  std::vector<std::vector<int>> permutations;
  if (star_tiles_.size() == 2) {
    permutations = {{0, 1}, {1, 0}};
  } else if (n == 2) {
    permutations = {{0, 1}, {1, 0}, {0, 2}, {2, 0}, {1, 2}, {2, 1}};
  } else {
    permutations = {{0, 1, 2}, {0, 2, 1}, {1, 0, 2},
                    {1, 2, 0}, {2, 0, 1}, {2, 1, 0}};
  }

  std::vector<std::string> regexes;
  for (const std::vector<int>& pmtn : permutations) {
    std::string rgx = ".*";
    rgx.push_back(star_tiles_[pmtn[0]]->letter());
    for (int i = 1; i < n; ++i) {
      int gap = std::abs(star_tiles_[pmtn[i - 1]]->col() -
                         star_tiles_[pmtn[i]]->col());
      if (gap > 0) --gap;
      absl::StrAppend(&rgx, ".{", gap, ",}");
      rgx.push_back(star_tiles_[pmtn[i]]->letter());
    }
    absl::StrAppend(&rgx, ".*");
    regexes.push_back(rgx);
  }

  return absl::StrJoin(regexes, "|",
                       [](std::string* out, const std::string& in) {
                         absl::StrAppend(out, "(", in, ")");
                       });
}

std::string Grid::VisualizePath(const Path& path) const {
  std::vector<Point> affected_points = PointsRemovedBy(path);
  std::vector<std::string> board = AsCharMatrix();
  for (int r = 0; r < board.size(); ++r) {
    for (int c = 0; c < board[r].size(); ++c) {
      Point p = {r, c};
      // In these cases, the character there is already correct.
      if (board[r][c] == kEmptySpaceLetter || path.contains(p)) continue;

      if (absl::c_contains(affected_points, p)) {
        board[r][c] = kAffectedSpaceLetter;
      } else {
        board[r][c] = kBlankTileLetter;
      }
    }
  }
  std::reverse(board.begin(), board.end());
  return absl::StrJoin(board, "\n");
}

absl::Status Grid::reset() {
  while (!tile_removal_history_.empty()) {
    if (absl::Status s = RevertLastClear(); !s.ok()) return s;
  }
  return absl::OkStatus();
}

absl::Status Grid::ClearPath(const Path& path) {
  std::vector<std::shared_ptr<Tile>> removed_tiles = TilesRemovedBy(path);
  tile_removal_history_.push_back(removed_tiles);

  for (const std::shared_ptr<Tile>& tile : removed_tiles) {
    auto [row, col] = tile->coords();

    // Possibly remove it from `star_tiles_`.
    if (tile->is_star()) {
      auto it = std::find(star_tiles_.begin(), star_tiles_.end(), tile);
      if (it == star_tiles_.end())
        return absl::InvalidArgumentError(
            "tile is a star tile, but is not contained in star_tiles_.");
      star_tiles_.erase(it);
    }

    // Possibly remove it from `letter_map_` and `column_letter_counts_`.
    if (!tile->is_blank()) {
      char l = tile->letter();
      letter_map_[l].erase(tile);
      if (absl::StatusOr<int> s = column_letter_counts_[col].RemoveLetter(l);
          !s.ok())
        return s.status();
    }

    // Shift the coordinates of all tiles above it in the column down by one.
    std::vector<std::shared_ptr<Tile>>& column = tiles_[col];
    for (int r = row + 1; r < kNumRows; ++r) {
      std::shared_ptr<Tile> curr_tile = column[r];
      if (curr_tile == nullptr) break;
      if (absl::Status s = curr_tile->Drop(1); !s.ok()) return s;
    }

    // Remove the tile itself, and insert a nullptr at the end of the column.
    column.erase(column.begin() + row);
    column.push_back(nullptr);
  }
  return absl::OkStatus();
}

absl::Status Grid::RevertLastClear() {
  if (tile_removal_history_.empty())
    return absl::FailedPreconditionError(
        "Grid has not been altered from its initial state");

  std::vector<std::shared_ptr<Tile>> removed_tiles =
      tile_removal_history_.back();
  // To ensure that we place tiles back in the correct places, we re-insert them
  // from lowest to highest, one column at a time.
  std::reverse(removed_tiles.begin(), removed_tiles.end());

  for (const std::shared_ptr<Tile>& tile : removed_tiles) {
    // Readd the tile to `tiles_`, and remove a `nullptr` from the end.
    auto [row, col] = tile->coords();
    std::vector<std::shared_ptr<Tile>>& column = tiles_[col];
    column.insert(column.begin() + row, tile);
    column.pop_back();

    // Possibly add it back to `star_tiles_`.
    if (tile->is_star()) star_tiles_.push_back(tile);

    // Possibly readd it to `letter_map_` and `column_letter_counts_`.
    if (!tile->is_blank()) {
      char l = tile->letter();
      letter_map_[l].insert(tile);
      if (absl::StatusOr<int> s = column_letter_counts_[col].AddLetter(l);
          !s.ok())
        return s.status();
    }

    // Shift the coordinates of all tiles above it in the column up by one.
    for (int r = row + 1; r < kNumRows; ++r) {
      std::shared_ptr<Tile> curr_tile = column[r];
      if (curr_tile == nullptr) break;
      if (absl::Status s = curr_tile->Drop(-1); !s.ok()) return s;
    }
  }

  tile_removal_history_.pop_back();
  return absl::OkStatus();
}

}  // namespace puzzmo::spelltower