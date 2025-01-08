#ifndef point_h
#define point_h

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

} // namespace puzzmo

#endif