#include "spelltower_board.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>

#include "absl/strings/str_cat.h"

namespace puzzmo::spelltower {
namespace {

const absl::flat_hash_map<char, int> kLetterValueMap(
    {{'a', 1}, {'b', 4}, {'c', 4},  {'d', 3}, {'e', 1}, {'f', 5}, {'g', 3},
     {'h', 5}, {'i', 1}, {'j', 9},  {'k', 6}, {'l', 2}, {'m', 4}, {'n', 2},
     {'o', 1}, {'p', 4}, {'q', 12}, {'r', 2}, {'s', 1}, {'t', 2}, {'u', 1},
     {'v', 5}, {'w', 5}, {'x', 9},  {'y', 5}, {'z', 11}});

// We sort the points by row, and within a row, from highest to lowest. This
// way, if we iterate over them and remove some, the rest won't be altered.
auto point_comparator = [](const Point &lhs, const Point &rhs) {
  return (lhs.row != rhs.row ? lhs.row < rhs.row : lhs.col > rhs.col);
};

// Very simple helper method
int EmptyRowsBetween(const Point &p1, const Point &p2) {
  int dist = std::abs(p1.row - p2.row);
  return dist == 0 ? 0 : dist - 1;
}

}  // namespace

SpelltowerBoard::SpelltowerBoard(const std::vector<std::string> &board)
    : board_(board), rows_(board.size()) {
  cols_ = 0;
  for (const auto &row : board_) {
    if (cols_ < row.size()) cols_ = row.size();
  }
  for (auto &row : board_) {
    while (row.size() < cols_) {
      row.push_back(' ');
    }
  }

  for (int r = 0; r < rows_; ++r) {
    for (int c = 0; c < cols_; ++c) {
      char ltr = char_at(r, c);
      char lowercase = std::tolower(ltr);
      if (std::isupper(ltr)) {
        stars_.push_back({r, c});
        board_[r][c] = lowercase;
      }
      if (std::isalpha(lowercase)) letter_map_[lowercase].push_back({r, c});
    }
  }
  std::sort(stars_.begin(), stars_.end(), point_comparator);
};

char SpelltowerBoard::char_at(const Point &p) const {
  return SpelltowerBoard::char_at(p.row, p.col);
}

char SpelltowerBoard::char_at(int row, int col) const {
  if (!HasPoint(row, col)) return '*';
  return board_[row][col];
}

std::vector<std::string> SpelltowerBoard::board() const { return board_; }

std::vector<Point> SpelltowerBoard::column(int col) const {
  std::vector<Point> v;
  if (col < 0 || col >= max_cols_) return v;
  for (int row = 0; row < max_rows_; ++row) {
    Point p = {row, col};
    if (char_at(p) == ' ') continue;
    v.push_back(p);
  }
  return v;
}

std::vector<Point> SpelltowerBoard::row(int row) const {
  std::vector<Point> v;
  if (row < 0 || row >= max_rows_) return v;
  for (int col = 0; col < max_cols_; ++col) {
    Point p = {row, col};
    if (char_at(p) == ' ') continue;
    v.push_back(p);
  }
  return v;
}

std::vector<Point> SpelltowerBoard::points() const {
  std::vector<Point> v;
  for (int r = 0; r < max_rows_; ++r) {
    for (int c = 0; c < max_cols_; ++c) {
      Point p = {r, c};
      if (char_at(p) == ' ') continue;
      v.push_back(p);
    }
  }
  return v;
}

bool SpelltowerBoard::is_star_at(const Point &p) const {
  return absl::c_contains(stars_, p);
}
bool SpelltowerBoard::is_star_at(int row, int col) const {
  return is_star_at({row, col});
}

char &SpelltowerBoard::operator[](Point p) { return board_[p.row][p.col]; }
const char &SpelltowerBoard::operator[](Point p) const {
  return board_[p.row][p.col];
}
std::string &SpelltowerBoard::operator[](int row) { return board_[row]; }
const std::string &SpelltowerBoard::operator[](int row) const {
  return board_[row];
}

bool SpelltowerBoard::HasPoint(const Point &p) const {
  return SpelltowerBoard::HasPoint(p.row, p.col);
}

bool SpelltowerBoard::HasPoint(int row, int col) const {
  return (row >= 0 && row < rows_ && col >= 0 && col < cols_);
}

void SpelltowerBoard::ClearPoint(const Point &p) {
  JustClearPoint(p);
  RegenLetterMap();
}

void SpelltowerBoard::ClearPath(const SpelltowerPath &path) {
  absl::flat_hash_set<Point> affected = AffectedSquares(path);
  absl::flat_hash_map<int, std::vector<Point>> affected_in_order;
  for (const Point &p : affected) affected_in_order[p.row].push_back(p);
  for (auto &[k, v] : affected_in_order) {
    std::sort(v.begin(), v.end(), point_comparator);
    for (const Point &p : v) JustClearPoint(p);
  }
  RegenLetterMap();
}

absl::flat_hash_set<Point> SpelltowerBoard::AffectedSquares(
    const SpelltowerPath &path) const {
  absl::flat_hash_set<Point> affected;

  // Determine the points outside of the path that will be removed, as they will
  // be involved in scoring.
  for (const Point &p : path.points()) {
    affected.insert(p);
    char c = char_at(p);

    // Red tiles
    if (c == 'j' || c == 'q' || c == 'x' || c == 'z') {
      for (const Point &cleared_pt : column(p.col)) {
        affected.insert(cleared_pt);
      }
    }

    // Adjacent tiles if path is long enough.
    if (path.size() >= 5) {
      for (const Point &vvnn : ValidVonNeumannNeighbors(p)) {
        affected.insert(vvnn);
      }
    }
  }
  return affected;
}

absl::flat_hash_set<Point> SpelltowerBoard::ValidVonNeumannNeighbors(
    const Point &p) const {
  absl::flat_hash_set<Point> neighbors = p.VonNeumannNeighbors();
  absl::erase_if(neighbors, [this](Point n) { return !HasPoint(n); });
  return neighbors;
}

absl::flat_hash_set<Point> SpelltowerBoard::ValidMooreNeighbors(
    const Point &p) const {
  absl::flat_hash_set<Point> neighbors = p.MooreNeighbors();
  absl::erase_if(neighbors, [this](Point n) {
    return !HasPoint(n) || !std::isalpha(board_[n.row][n.col]);
  });
  return neighbors;
}

int SpelltowerBoard::NumRows() const { return rows_; }
int SpelltowerBoard::NumCols() const { return cols_; }
int SpelltowerBoard::NumStars() const { return stars_.size(); }
std::string SpelltowerBoard::StarLetters() const {
  std::string s;
  for (const auto &star : stars_) {
    s += char_at(star);
  }
  return s;
}

absl::flat_hash_set<std::string> SpelltowerBoard::GetAllStarRegexes() const {
  if (stars_.empty()) return {};

  std::vector<Point> star_vec(stars_);
  absl::flat_hash_set<std::string> regex_set;
  do {
    std::string s(1, char_at(star_vec[0]));
    for (int i = 1; i < star_vec.size(); ++i) {
      int d = EmptyRowsBetween(star_vec[i - 1], star_vec[i]);
      if (d == 0) {
        absl::StrAppend(&s, ".*", std::string(1, char_at(star_vec[i])));
      } else {
        absl::StrAppend(&s, ".{", d, ",}",
                        std::string(1, char_at(star_vec[i])));
      }
    }
    regex_set.insert(s);
  } while (std::next_permutation(star_vec.begin(), star_vec.end(),
                                 point_comparator));
  return regex_set;
}

std::vector<LetterCount> SpelltowerBoard::GetRowLetterCounts() const {
  std::vector<LetterCount> row_letter_counts;
  for (const auto &row : board_) {
    row_letter_counts.push_back(LetterCount(row));
  }
  return row_letter_counts;
}

std::vector<Point> SpelltowerBoard::StarLocations() const { return stars_; }

int SpelltowerBoard::ScorePath(const SpelltowerPath &path) const {
  absl::flat_hash_set<Point> affected = AffectedSquares(path);

  // Sum the values of all affected points, multiply by path.size(), and
  // multiply again by the number of stars used (plus one).
  int score = 0;
  for (const Point &p : affected) {
    if (char c = char_at(p); std::isalpha(c)) {
      score += kLetterValueMap.at(c);
    }
  }
  score *= path.size();
  return score *= (1 + path.num_stars());
}

void SpelltowerBoard::JustClearPoint(const Point &p) {
  board_[p.row].erase(board_[p.row].begin() + p.col);
  board_[p.row].push_back(' ');
  for (int i = stars_.size() - 1; i >= 0; --i) {
    if (stars_[i].row != p.row) continue;
    if (stars_[i].col == p.col) stars_.erase(stars_.begin() + i);
    if (stars_[i].col > p.col) --stars_[i].col;
  }
}

void SpelltowerBoard::RegenLetterMap() {
  letter_map_.clear();
  for (int r = 0; r < rows_; ++r) {
    for (int c = 0; c < cols_; ++c) {
      char l = char_at(r, c);
      if (std::isalpha(l)) letter_map_[l].push_back({r, c});
    }
  }
}

}  // namespace puzzmo::spelltower