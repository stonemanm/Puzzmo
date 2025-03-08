#ifndef point_h
#define point_h

#include <utility>

#include "absl/container/flat_hash_set.h"
#include "absl/strings/str_format.h"

namespace puzzmo {

// A helper object storing a pair of row/column indices.
struct Point {
  int row;
  int col;

  // Returns the 4 orthogonally-adjacent neighbors of this point.
  absl::flat_hash_set<Point> VonNeumannNeighbors() const;

  // Returns the 8 surrounding neighbors of this point.
  absl::flat_hash_set<Point> MooreNeighbors() const;

  template <typename H>
  friend H AbslHashValue(H h, const Point &p) {
    return H::combine(std::move(h), p.row, p.col);
  }

  template <typename Sink>
  friend void AbslStringify(Sink &sink, const Point &p) {
    absl::Format(&sink, "(%d,%d)", p.row, p.col);
  }

  Point &operator+=(const Point &rhs);
};

bool operator==(const Point &lhs, const Point &rhs);
bool operator!=(const Point &lhs, const Point &rhs);
Point operator+(const Point &lhs, const Point &rhs);

}  // namespace puzzmo

#endif