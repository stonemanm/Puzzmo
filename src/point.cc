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

Point &Point::operator+=(const Point &rhs) {
  row += rhs.row;
  col += rhs.col;
  return *this;
}

bool operator==(const Point &lhs, const Point &rhs) {
  return lhs.row == rhs.row && lhs.col == rhs.col;
}
bool operator!=(const Point &lhs, const Point &rhs) { return !(lhs == rhs); }
Point operator+(const Point &lhs, const Point &rhs) {
  Point result = lhs;
  result += rhs;
  return result;
}

}  // namespace puzzmo