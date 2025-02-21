#ifndef spelltower_path_h
#define spelltower_path_h

#include <string>
#include <vector>

#include "point.h"

namespace puzzmo {

class SpelltowerPath {
 public:
  SpelltowerPath() : simplified_board_(9) {}
  explicit SpelltowerPath(const std::vector<Point>& points);

  void pop_back();
  void push_back(const Point& p);

  // Functions linked to `points_`
  std::vector<Point> points() const;
  Point at(int i) const;
  bool empty() const;
  int size() const;

  // Functions linked to `num_below_`
  std::vector<int> num_below() const;
  int num_below(int i) const;

  // Functions linked to `simplified_board_`
  std::vector<std::string> simplified_board() const;
  std::string SimplifiedRow(int i) const;

  // Other functions
  std::vector<int> IndicesByColumn() const;

 private:
  std::vector<Point> points_;
  std::vector<int> num_below_;
  std::vector<std::string> simplified_board_;
};

}  // namespace puzzmo

#endif  // !spelltower_path_h