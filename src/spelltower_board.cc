#include "spelltower_board.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>

#include "absl/log/log.h"
#include "absl/strings/str_cat.h"

namespace puzzmo {
namespace {

// Very simple helper method
int EmptyRowsBetween(const Point &p1, const Point &p2) {
  int dist = std::abs(p1.row - p2.row);
  return dist == 0 ? 0 : dist - 1;
}

std::string StringifyPath(std::vector<Point> &path) {
  std::vector<std::string> board(9, "");
  for (int i = 0; i < path.size(); ++i) {
    Point p = path[i];
    while (p.col >= board[p.row].length()) board[p.row].push_back('*');
    board[p.row][p.col] = 'A' + i;
  }
  std::string s = "";
  for (int i = 0; i < board.size(); ++i) {
    absl::StrAppend(&s, i, "[", board[i], "\n");
  }
  return s;
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
  std::sort(stars_.begin(), stars_.end());
};

char SpelltowerBoard::char_at(const Point &p) const {
  return SpelltowerBoard::char_at(p.row, p.col);
}

char SpelltowerBoard::char_at(int row, int col) const {
  if (!HasPoint(row, col)) return '*';
  return board_[row][col];
}

bool SpelltowerBoard::HasPoint(const Point &p) const {
  return SpelltowerBoard::HasPoint(p.row, p.col);
}

bool SpelltowerBoard::HasPoint(int row, int col) const {
  return (row >= 0 && row < rows_ && col >= 0 && col < cols_);
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
  absl::erase_if(neighbors, [this](Point n) { return !HasPoint(n); });
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
  } while (std::next_permutation(star_vec.begin(), star_vec.end()));
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

int SpelltowerBoard::Score(const absl::flat_hash_set<Point> &path) const {
  absl::flat_hash_set<Point> affected;
  int star_tiles = 0;
  for (const Point p : path) {
    affected.insert(p);
    if (absl::c_contains(stars_, p)) ++star_tiles;

    char c = char_at(p);
    if (c == 'j' || c == 'q' || c == 'x' || c == 'z') {
      for (int row = 0; row < rows_; ++row) {
        affected.insert({.row = row, .col = p.col});
      }
    }
    if (path.size() < 5) continue;

    for (const Point n : ValidVonNeumannNeighbors(p)) {
      affected.insert(n);
    }
  }

  int score = 0;
  for (const Point p : affected) {
    char c = char_at(p);
    if (c == ' ' || c == '*') continue;
    score += kLetterScores[c - 'a'];
  }
  score *= path.size();
  return score *= (1 + star_tiles);
}

std::vector<std::string> SpelltowerBoard::MightHaveAllStarWords(
    const std::vector<std::string> &words) const {
  std::vector<std::string> filtered_words;
  for (const auto &wd : words) {
    if (MightHaveAllStarWord(wd)) {
      filtered_words.push_back(wd);
      // TEMP
      if (filtered_words.size() >= 10) break;
    }
  }
  return filtered_words;
}

bool SpelltowerBoard::MightHaveAllStarWord(absl::string_view word) const {
  std::vector<LetterCount> row_letter_counts = GetRowLetterCounts();
  std::vector<Point> path;
  if (DFS(word, 0, row_letter_counts, path)) {
    StringifyPath(path);
    return true;
  }
  return false;
}

bool SpelltowerBoard::DFS(absl::string_view word, int i,
                          std::vector<LetterCount> &row_letter_counts,
                          std::vector<Point> &path) const {
  // Check if we have a solution
  if (i >= word.length()) {
    for (const Point &star : stars_) {
      if (!absl::c_contains(path, star)) return false;
    }
    return IsPathPossible(path);
  }

  // For every possible choice in the current position...
  char c = word[i];
  for (const Point &p : letter_map_.at(c)) {
    if ((!path.empty() && std::abs(p.row - path.back().row) > 1) ||
        !row_letter_counts[p.row].contains(c))
      continue;

    // Make the choice
    path.push_back(p);
    if (auto e = row_letter_counts[p.row].RemoveLetter(c); !e.ok()) {
      LOG(ERROR) << e.status();
      return false;
    }

    // Use recursion to solve from the new position
    if (DFS(word, i + 1, row_letter_counts, path)) return true;

    // Unmake the choice
    path.pop_back();
    if (auto e = row_letter_counts[p.row].AddLetter(c); !e.ok()) {
      LOG(ERROR) << e.status();
      return false;
    }
  }
  return false;
}

bool SpelltowerBoard::IsPathPossible(std::vector<Point> &path) const {
  // Group the points in the path by row
  std::vector<std::vector<int>> simplified_board(max_rows_);
  std::vector<int> min_heights(path.size());
  for (int index = 0; index < path.size(); ++index)
    simplified_board[path[index].row].push_back(index);

  // Sorts each group by column, then determines the lowest each point can go
  for (int row = 0; row < simplified_board.size(); ++row) {
    std::vector<int> &row_vec = simplified_board[row];
    std::sort(row_vec.begin(), row_vec.end(),
              [path](int a, int b) { return path[a].col < path[b].col; });
    for (int i = 0; i < row_vec.size(); ++i) min_heights[row_vec[i]] = i;
  }

  // Check for interrupted columns (or "A-C-B columns")
  for (int i = 1; i < path.size(); ++i) {
    if (path[i - 1].row == path[i].row &&
        std::abs(min_heights[i - 1] - min_heights[i]) > 1)
      return false;
  }

  // Align the path from the bottom up
  std::vector<int> l_to_h(path.size());
  std::iota(l_to_h.begin(), l_to_h.end(), 0);
  bool aligned = false;
  while (!aligned) {
    // Start with the lowest point
    std::sort(l_to_h.begin(), l_to_h.end(),
              [path](int a, int b) { return path[a].col < path[b].col; });
    for (int i = 0; i < l_to_h.size(); ++i) {
      int path_idx = l_to_h[i];
      Point &curr = path[path_idx];
      if (path_idx > 0) {
        Point &prev = path[path_idx - 1];
        if (!curr.MooreNeighbors().contains(prev)) {
          if (!UpdatePath(path, path_idx - 1, min_heights, simplified_board))
            return false;
          break;  // Restarts the for loop.
        }
      }
      if (path_idx < path.size() - 1) {
        Point &next = path[path_idx + 1];
        if (!curr.MooreNeighbors().contains(next)) {
          if (!UpdatePath(path, path_idx, min_heights, simplified_board))
            return false;
          break;  // Restarts the for loop.
        }
      }
      // If we've made it here, this point (and all below it) can reach both of
      // their neighbors! If this is the final loop, set aligned = true so we
      // can break free
      if (i == l_to_h.size() - 1) aligned = true;
    }
    if (aligned) break;
  }
  return true;
}

bool SpelltowerBoard::UpdatePath(
    std::vector<Point> &path, int l, const std::vector<int> &min_col,
    const std::vector<std::vector<int>> &simplified_board) const {
  int lo = (path[l].col < path[l + 1].col) ? l : l + 1;
  int hi = (path[l + 1].col < path[l].col) ? l : l + 1;
  if (path[lo].col + 1 < min_col[hi]) return false;

  // Lower hi to one column above lo. Lowers any points above hi as well.
  int shift = path[hi].col - path[lo].col - 1;
  std::vector<int> row_items = simplified_board[path[hi].row];

  // Best way to do the shift is highest to lowest, I think.
  bool above_hi = true;
  for (int i = row_items.size() - 1; i >= 0; --i) {
    int cur_idx = row_items[i];
    // Anything above the point being lowered needs to drop the same amount.
    if (above_hi) {
      path[cur_idx].col -= shift;
      if (cur_idx == hi) above_hi = false;
      continue;
    }
    // If we're below the point being lowered, we lower it as little as needed
    // to stay below the point above it.
    // It's safe to refer to row_items[i+1].
    path[cur_idx].col = path[row_items[i + 1]].col - 1;
  }
  return true;
}

}  // namespace puzzmo