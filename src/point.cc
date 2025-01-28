#include "point.h"

namespace puzzmo {

absl::flat_hash_set<Point> Point::VonNeumannNeighbors() const {
  return {{row, col + 1}, {row + 1, col}, {row, col - 1}, {row - 1, col}};
}

absl::flat_hash_set<Point> Point::MooreNeighbors() const {
  return {{row, col + 1},     {row + 1, col + 1}, {row + 1, col},
          {row + 1, col - 1}, {row, col - 1},     {row - 1, col - 1},
          {row - 1, col},     {row - 1, col + 1}};
}

bool Point::operator==(const Point &other) const {
  return row == other.row && col == other.col;
}

Point Point::operator+(const Point &other) const {
  return {row + other.row, col + other.col};
}

} // namespace puzzmo