#ifndef point_h
#define point_h

#include <utility>

#include "absl/container/flat_hash_set.h"

namespace puzzmo {

// A helper object storing a pair of row/column indices.
struct Point {
  int row;
  int col;

  // Returns the 4 orthogonally-adjacent neighbors of this point.
  absl::flat_hash_set<Point> VonNeumannNeighbors() const;

  // Returns the 8 surrounding neighbors of this point.
  absl::flat_hash_set<Point> MooreNeighbors() const;

  bool operator==(const Point &other) const;
  bool operator<(const Point &other) const;
  Point operator+(const Point &other) const;

  template <typename H> friend H AbslHashValue(H h, const Point &p) {
    return H::combine(std::move(h), p.row, p.col);
  }
};

} // namespace puzzmo

#endif