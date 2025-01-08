#include "point.h"

namespace puzzmo {

bool Point::operator==(const Point &other) const {
  return row == other.row && col == other.col;
}

Point Point::operator+(const Point &other) const {
  return {row + other.row, col + other.col};
}

} // namespace puzzmo