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
                 [*this](Point p) { return !IsPointInRange(p); });

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
  absl::erase_if(accessible_tiles, [path](std::shared_ptr<Tile> tile) {
    return tile->is_blank() || path.contains(tile->coords());
  });
  return accessible_tiles;
}

absl::flat_hash_set<std::shared_ptr<Tile>> Grid::TilesAffectedBy(
    const std::shared_ptr<Tile>& tile) const {
  if (tile == nullptr) return {};

  absl::flat_hash_set<Point> von_neumann_neighbors =
      tile->coords().VonNeumannNeighbors();
  absl::erase_if(von_neumann_neighbors,
                 [*this](Point p) { return !IsPointInRange(p); });

  absl::flat_hash_set<std::shared_ptr<Tile>> accessible_tiles;
  for (const Point& p : von_neumann_neighbors) {
    std::shared_ptr<Tile> tile = tiles_[p.col][p.row];
    if (tile == nullptr) continue;
    accessible_tiles.insert(tile);
  }
  return accessible_tiles;
}

absl::flat_hash_set<Point> Grid::PointsRemovedBy(const Path& path) const {
  absl::flat_hash_set<Point> affected_points;

  for (const std::shared_ptr<Tile>& tile : path.tiles()) {
    // We can be confident that `path` does not contain any nullptr entries.
    affected_points.insert(tile->coords());
    absl::flat_hash_set<Point> vnn = tile->coords().VonNeumannNeighbors();
    if (path.size() < 5) {
      // If path size is less than 5, we still want to eliminate adjacent
      // blank tiles.
      absl::erase_if(vnn, [*this](Point p) {
        return !IsPointInRange(p) || !tiles_[p.col][p.row]->is_blank();
      });
    }
    affected_points.insert(vnn.begin(), vnn.end());
    if (tile->is_rare()) {
      for (int c = 0; c < kNumCols; ++c)
        affected_points.insert({tile->row(), c});
    }
  }
  absl::erase_if(affected_points,
                 [*this](Point p) { return !IsPointInRange(p); });
  return affected_points;
}

absl::flat_hash_set<std::shared_ptr<Tile>> Grid::TilesRemovedBy(
    const Path& path) const {
  absl::flat_hash_set<std::shared_ptr<Tile>> accessible_tiles;

  absl::flat_hash_set<Point> affected_points = PointsRemovedBy(path);
  for (const Point& p : affected_points) {
    std::shared_ptr<Tile> tile = tiles_[p.col][p.row];
    if (tile == nullptr) continue;
    accessible_tiles.insert(tile);
  }
  return accessible_tiles;
}

std::vector<std::string> Grid::AsStringVector() const {
  std::vector<std::string> v;
  for (int r = 0; r < kNumRows; ++r) {
    std::vector<std::shared_ptr<Tile>> grid_row = row(r);

    int last_c = grid_row.size();
    while (grid_row[last_c - 1] == nullptr && last_c > 0) --last_c;
    if (last_c == 0) break;

    std::string s;
    for (int c = 0; c <= last_c; ++c) {
      std::shared_ptr<Tile> tile = tiles_[c][r];
      if (tile == nullptr)
        absl::StrAppend(&s, std::string(1, kEmptySpaceLetter));
      else
        absl::StrAppend(&s, *tile);
    }
    v.push_back(s);
  }
  return v;
}

std::string Grid::VisualizePath(const Path& path) const {
  absl::flat_hash_set<Point> points_removed_by = PointsRemovedBy(path);
  std::vector<std::string> board = AsStringVector();
  for (int r = 0; r < board.size(); ++r) {
    for (int c = 0; c < kNumCols; ++c) {
      if (board[r][c] == kEmptySpaceLetter) continue;

      Point p = {r, c};
      if (path.contains(p)) continue;

      if (points_removed_by.contains(p)) {
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