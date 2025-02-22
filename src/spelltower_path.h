#ifndef spelltower_path_h
#define spelltower_path_h

#include <string>
#include <vector>

#include "absl/strings/str_cat.h"
#include "point.h"

namespace puzzmo {

// Manages a vector of points, calculating things like num_below in the process.
// Does NOT require that adjacent points be Moore neighbors.
class SpelltowerPath {
 public:
  SpelltowerPath() : simplified_board_(9) {}
  explicit SpelltowerPath(const std::vector<Point> &points);

  // Removes the last point in the path and adjusts data accordingly.
  void pop_back();

  // Adds point p to the end of the path and adjusts data accordingly.
  void push_back(const Point &p);

  // Functions linked to `points_`
  std::vector<Point> points() const;
  Point &operator[](int i);
  const Point &operator[](int i) const;
  Point &back();
  bool empty() const;
  int size() const;

  // Functions linked to `num_below_`
  std::vector<int> num_below() const;
  int num_below(int i) const;

  // Functions linked to `simplified_board_`
  std::vector<std::vector<int>> simplified_board() const;
  std::vector<int> SimplifiedRow(int i) const;

  // Other functions
  std::vector<int> IndicesByColumn() const;

 private:
  // The vector of points in the path.
  std::vector<Point> points_;

  // For the point at the same index in points_, contains the number of points
  // in this path that are below that point in the same column. This can be used
  // to establish a minimum possible column to which that point can be moved.
  std::vector<int> num_below_;

  // A projection of the board
  std::vector<std::vector<int>> simplified_board_;

  /** * * * * * * * * *
   * Abseil functions *
   * * * * * * * * * **/

  // Allows hashing of SpelltowerPath.
  template <typename H>
  friend H AbslHashValue(H h, const SpelltowerPath &path) {
    return H::combine(std::move(h), path.points_, path.num_below_,
                      path.simplified_board_);
  }

  // Allows easy conversion of SpelltowerPath to string.
  template <typename Sink>
  friend void AbslStringify(Sink &sink, const SpelltowerPath &path) {
    std::vector<std::string> board(9, "");
    for (int row = 0; row < 9; ++row) {
      for (int idx : path.SimplifiedRow(row)) {
        Point p = path[idx];
        while (board[row].length() < p.col) board[row].push_back('*');
        board[row].push_back('A' + idx);
      }
    }
    sink.Append(absl::StrCat("\n0[", board[0], "\n1[", board[1], "\n2[",
                             board[2], "\n3[", board[3], "\n4[", board[4],
                             "\n5[", board[5], "\n6[", board[6], "\n7[",
                             board[7], "\n8[", board[8]));
  }
};

}  // namespace puzzmo

#endif  // !spelltower_path_h