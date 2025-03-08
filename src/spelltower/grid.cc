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
  absl::flat_hash_set<std::shared_ptr<Tile>> affected_tiles =
      TilesRemovedBy(path);

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
  return std::all_of(tiles_.begin(), tiles_.end(), [](const auto& column) {
    return absl::c_count_if(
               column, [](const auto& tile) { return tile != nullptr; }) <= 2;
  });
}

bool Grid::FullClear() const {
  return std::all_of(tiles_.begin(), tiles_.end(), [](const auto& column) {
    return std::all_of(column.begin(), column.end(),
                       [](const auto& tile) { return tile == nullptr; });
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

absl::flat_hash_set<std::shared_ptr<Tile>> Grid::TilesAffectedBy(
    const std::shared_ptr<Tile>& tile) const {
  absl::flat_hash_set<std::shared_ptr<Tile>> affected_tiles;

  absl::flat_hash_set<Point> affected_points = PointsAffectedBy(tile);
  for (const Point& p : affected_points) {
    std::shared_ptr<Tile> tile = tiles_[p.col][p.row];
    if (tile == nullptr) continue;
    affected_tiles.insert(tile);
  }
  return affected_tiles;
}

absl::flat_hash_set<Point> Grid::PointsRemovedBy(const Path& path) const {
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
    if (path.size() < 5) {
      absl::erase_if(points_affected_by, [*this](const Point& p) {
        return !tiles_[p.col][p.row]->is_blank();
      });
    }
    affected_points.insert(points_affected_by.begin(),
                           points_affected_by.end());
  }
  return affected_points;
}

absl::flat_hash_set<std::shared_ptr<Tile>> Grid::TilesRemovedBy(
    const Path& path) const {
  absl::flat_hash_set<std::shared_ptr<Tile>> accessible_tiles;

  absl::flat_hash_set<Point> affected_points = PointsRemovedBy(path);
  for (const Point& p : affected_points) {
    // This should never be `nullptr` due to checks in `PointsRemovedBy()`.
    std::shared_ptr<Tile> tile = tiles_[p.col][p.row];
    accessible_tiles.insert(tile);
  }
  return accessible_tiles;
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

std::string Grid::VisualizePath(const Path& path) const {
  absl::flat_hash_set<Point> affected_points = PointsRemovedBy(path);
  std::vector<std::string> board = AsCharMatrix();
  for (int r = 0; r < board.size(); ++r) {
    for (int c = 0; c < board[r].size(); ++c) {
      Point p = {r, c};
      // In these cases, the character there is already correct.
      if (board[r][c] == kEmptySpaceLetter || path.contains(p)) continue;

      if (affected_points.contains(p)) {
        board[r][c] = kAffectedSpaceLetter;
      } else {
        board[r][c] = kBlankTileLetter;
      }
    }
  }
  std::reverse(board.begin(), board.end());
  return absl::StrJoin(board, "\n");
}

absl::Status Grid::ClearPath(const Path& path) {
  absl::flat_hash_set<std::shared_ptr<Tile>> affected = TilesRemovedBy(path);
  for (const auto& tile : affected) {
    if (auto s = ClearTile(tile); !s.ok()) return s;
  }
  return absl::OkStatus();
}

absl::Status Grid::ClearTile(const std::shared_ptr<Tile>& tile) {
  if (tile == nullptr)
    return absl::InvalidArgumentError(
        "Cannot pass nullptr to Grid::CloseTile().");
  if (!IsPointInRange(tile->coords()))
    return absl::InvalidArgumentError("Tile is not on board.");
  auto [row, col] = tile->coords();

  // Remove it from the other data members.
  if (tile->is_star()) {
    auto it = std::find(star_tiles_.begin(), star_tiles_.end(), tile);
    if (it == star_tiles_.end())
      return absl::InvalidArgumentError(
          "tile is a star tile, but is not contained in star_tiles_.");
    star_tiles_.erase(it);
  }
  if (!tile->is_blank()) {
    char l = tile->letter();
    letter_map_[l].erase(tile);
    if (auto s = column_letter_counts_[col].RemoveLetter(l); !s.ok()) {
      return s.status();
    }
  }

  // Shift the coordinates of all tiles above it in the column down by one.
  std::vector<std::shared_ptr<Tile>>& column = tiles_[col];
  for (int r = row + 1; r < kNumRows; ++r) {
    if (column[r] == nullptr) break;
    if (auto s = column[r]->Drop(1); !s.ok()) return s;
  }

  // Remove the tile itself, and insert a nullptr at the end of the column.
  column.erase(column.begin() + row);
  column.push_back(nullptr);
  return absl::OkStatus();
}

}  // namespace puzzmo::spelltower