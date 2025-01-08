#ifndef point_h
#define point_h

#include <utility>

namespace puzzmo {

// A helper object storing a pair of row/column indices.
struct Point {
  int row;
  int col;

  bool operator==(const Point &other) const;

  Point operator+(const Point &other) const;

  template <typename H> friend H AbslHashValue(H h, const Point &p) {
    return H::combine(std::move(h), p.row, p.col);
  }
};

// Add to a point to get to the 8 surrounding spots around that point.
const Point kAdjacentCoords[] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, 1},
                                 {1, 1},   {1, 0},  {1, -1}, {0, -1}};

// Add to a point to get to the 4 spots around that point.
const Point kDPad[] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};

} // namespace puzzmo

#endif