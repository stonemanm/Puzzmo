#include "grid.h"

#include <algorithm>

namespace puzzmo::spelltower {
namespace {

// Returns the number of non-nullptr entries before the first nullptr.
int NumTilesInColumn(const std::vector<std::shared_ptr<Tile>>& column) {
  int n = 0;
  while (n < column.size() && column[n] != nullptr) ++n;
  return n;
}

}  // namespace

Grid::Grid(const std::vector<std::string>& grid)
    : tiles_(num_cols_), column_letter_counts_(num_cols_) {
  for (int r = 0; r < num_rows_; ++r) {
    int in_r = (grid.size() - 1) - r;
    for (int c = 0; c < num_cols_; ++c) {
      if (in_r < 0 || c >= grid[in_r].size() ||
          grid[in_r][c] == kEmptySpaceLetter) {
        tiles_[c].push_back(nullptr);
        continue;
      }

      std::shared_ptr<Tile> tile = std::make_shared<Tile>(r, c, grid[in_r][c]);
      tiles_[c].push_back(tile);
      if (tile->is_star()) star_tiles_.push_back(tile);
      if (tile->is_blank()) continue;

      letter_map_[tile->letter()].insert(tile);
      (void)column_letter_counts_[c].AddLetter(tile->letter());
    }
  }
}

std::vector<std::shared_ptr<Tile>> Grid::row(int row) const {
  std::vector<std::shared_ptr<Tile>> v;
  for (int col = 0; col < num_cols_; ++col) {
    Point p = {row, col};
    v.push_back(IsPointInRange(p) ? tiles_[col][row] : nullptr);
  }
  return v;
}

bool Grid::AlmostThere() const {
  for (int c = 0; c < num_cols_; ++c) {
    if (NumTilesInColumn(tiles_[c]) > 2) return false;
  }
  return true;
}

bool Grid::FullClear() const {
  for (int c = 0; c < num_cols_; ++c) {
    if (NumTilesInColumn(tiles_[c]) > 0) return false;
  }
  return true;
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
  if (p.row < 0 || p.row >= num_rows_ || p.col < 0 || p.col >= num_cols_)
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

absl::flat_hash_set<std::shared_ptr<Tile>> Grid::TilesRemovedBy(
    const Path& path) const {
  absl::flat_hash_set<Point> affected_points;

  for (const std::shared_ptr<Tile>& tile : path.tiles()) {
    affected_points.insert(tile->coords());
    absl::flat_hash_set<Point> vnn = tile->coords().VonNeumannNeighbors();
    if (path.size() < 5) {
      // If path size is less than 5, we still want to eliminate adjacent blank
      // tiles.
      absl::erase_if(vnn, [*this](Point p) {
        return !IsPointInRange(p) || !tiles_[p.col][p.row]->is_blank();
      });
    }
    affected_points.insert(vnn.begin(), vnn.end());
    if (tile->is_rare()) {
      for (int c = 0; c < num_cols_; ++c)
        affected_points.insert({tile->row(), c});
    }
  }
  absl::erase_if(affected_points,
                 [*this](Point p) { return !IsPointInRange(p); });

  absl::flat_hash_set<std::shared_ptr<Tile>> accessible_tiles;
  for (const Point& p : affected_points) {
    std::shared_ptr<Tile> tile = tiles_[p.col][p.row];
    if (tile == nullptr) continue;
    accessible_tiles.insert(tile);
  }
  return accessible_tiles;
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
  for (int r = row + 1; r < num_rows_; ++r) {
    if (column[r] == nullptr) break;
    if (auto s = column[r]->Drop(1); !s.ok()) return s;
  }

  // Remove the tile itself, and insert a nullptr at the end of the column.
  column.erase(column.begin() + row);
  column.push_back(nullptr);
  return absl::OkStatus();
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

}  // namespace puzzmo::spelltower