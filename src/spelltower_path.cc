#include "spelltower_path.h"

#include <algorithm>

namespace puzzmo {

SpelltowerPath::SpelltowerPath(const std::vector<Point>& points)
    : simplified_board_(9) {
  for (const Point& p : points) {
    push_back(p);
  }
}

void SpelltowerPath::pop_back() {
  const int idx = points_.size() - 1;
  Point& p = points_[idx];

  std::vector<int>& simplified_row = simplified_board_[p.row];

  for (int i = num_below(idx) + 1; i < simplified_row.size(); ++i) {
    --num_below_[simplified_row[i]];
  }
  simplified_row.erase(simplified_row.begin() + num_below_[idx]);
  num_below_.pop_back();
  points_.pop_back();
}

void SpelltowerPath::push_back(const Point& p) {
  const int idx = points_.size();
  points_.push_back(p);

  std::vector<int>& simplified_row = simplified_board_[p.row];
  num_below_.push_back(simplified_row.size());
  simplified_row.push_back(idx);
  for (int i = simplified_row.size() - 1; i > 0; --i) {
    if (points_[simplified_row[i - 1]].col < points_[simplified_row[i]].col)
      break;
    ++num_below_[simplified_row[i - 1]];
    --num_below_[simplified_row[i]];
    std::swap(simplified_row[i - 1], simplified_row[i]);
  }
}

// Functions linked to `points_`
std::vector<Point> SpelltowerPath::points() const { return points_; }
Point& SpelltowerPath::operator[](int i) { return points_[i]; }
const Point& SpelltowerPath::operator[](int i) const { return points_[i]; }
Point& SpelltowerPath::back() { return points_.back(); }
bool SpelltowerPath::empty() const { return points_.empty(); }
int SpelltowerPath::size() const { return points_.size(); }

// Functions linked to `num_letters_below_`
int SpelltowerPath::num_below(int i) const { return num_below_[i]; }
std::vector<int> SpelltowerPath::num_below() const { return num_below_; }

// Functions linked to `simplified_board_`
std::vector<int> SpelltowerPath::SimplifiedRow(int row) const {
  return simplified_board_[row];
}
std::vector<std::vector<int>> SpelltowerPath::simplified_board() const {
  return simplified_board_;
}

// Other functions
std::vector<int> SpelltowerPath::IndicesByColumn() const {
  std::vector<int> indices(size());
  std::iota(indices.begin(), indices.end(), 0);
  std::sort(indices.begin(), indices.end(),
            [*this](int a, int b) { return points_[a].col < points_[b].col; });
  return indices;
}

}  // namespace puzzmo
