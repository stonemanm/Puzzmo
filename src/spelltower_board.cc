#include "spelltower_board.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <stdexcept>

#include "absl/log/log.h"
#include "absl/strings/str_cat.h"

namespace puzzmo {
namespace {

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

bool UpdatePath(std::vector<Point> &path, int l, int r,
                const std::vector<int> &min_col,
                const std::vector<std::vector<int>> &row_i_in_order) {
  int lo_idx = (path[l].col < path[r].col) ? l : r;
  int hi_idx = (path[r].col < path[l].col) ? l : r;
  Point &lo = path[lo_idx];
  Point &hi = path[hi_idx];
  if (lo.col + 1 < min_col[hi_idx]) return false;

  // Lower hi to one column above lo. Lowers any points above hi as well.
  int shift = hi.col - lo.col - 1;
  std::vector<int> row_items = row_i_in_order[hi.row];

  // Best way to do the shift is highest to lowest, I think.
  bool above_hi = true;
  for (int i = row_items.size() - 1; i >= 0; --i) {
    int cur_idx = row_items[i];
    // Anything above the point being lowered needs to drop the same amount.
    if (above_hi) {
      path[cur_idx].col -= shift;
      if (cur_idx == hi_idx) above_hi = false;
      continue;
    }
    // If we're below the point being lowered, we lower it as little as needed
    // to stay below the point above it.
    // It's safe to refer to row_items[i+1].
    path[cur_idx].col = path[row_items[i + 1]].col - 1;
  }
  return true;
}

bool IsPathPossible(std::vector<Point> &path) {
  // Sort the letters within the path by row, and determine the lowest column
  // they can ever reach.
  std::vector<std::vector<int>> rows(9);
  std::vector<int> min_col(path.size());
  for (int index = 0; index < path.size(); ++index) {
    std::vector<int> &row = rows[path[index].row];
    row.push_back(index);
  }

  for (int row = 0; row < 9; ++row) {
    std::sort(rows[row].begin(), rows[row].end(),
              [path](int a, int b) { return path[a].col < path[b].col; });
    for (int i = 0; i < rows[row].size(); ++i) {
      min_col[rows[row][i]] = i;
    }
  }

  // Check for same-column links with something between them
  for (int i = 1; i < path.size(); ++i) {
    if (path[i - 1].row != path[i].row) continue;
    if (std::abs(min_col[i - 1] - min_col[i]) > 1) return false;
  }

  std::vector<int> l_to_h(path.size());
  std::iota(l_to_h.begin(), l_to_h.end(), 0);
  bool aligned = false;
  while (!aligned) {
    // Reset the order of points.
    std::sort(l_to_h.begin(), l_to_h.end(),
              [path](int a, int b) { return path[a].col < path[b].col; });
    for (int i = 0; i < l_to_h.size(); ++i) {
      int path_idx = l_to_h[i];
      Point &curr = path[path_idx];
      if (path_idx > 0) {
        if (!curr.MooreNeighbors().contains(path[path_idx - 1])) {
          if (!UpdatePath(path, path_idx - 1, path_idx, min_col, rows))
            return false;
          break;  // Restarts the for loop.
        }
      }
      if (path_idx < path.size() - 1) {
        if (!curr.MooreNeighbors().contains(path[path_idx + 1])) {
          if (!UpdatePath(path, path_idx, path_idx + 1, min_col, rows))
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
  LOG(INFO) << "✔✔✔ Path is possible!";
  return true;
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

bool SpelltowerBoard::MightHaveWord(absl::string_view word) const {
  for (int row = 0; row < rows_; ++row) {
    if (DFS(word, 0, row)) return true;
  }
  return false;
}

bool SpelltowerBoard::MightHaveAllStarWord(absl::string_view word) const {
  std::vector<LetterCount> row_letter_counts = GetRowLetterCounts();
  std::vector<Point> path;
  std::vector<std::vector<Point>> paths;
  DFS(word, 0, row_letter_counts, path, paths);
  LOG(INFO) << word << " has " << paths.size() << " paths.";
  for (auto &path : paths) {
    if (IsPathPossible(path)) {
      StringifyPath(path);
      return true;
    }
  }
  return false;
}

std::vector<std::string> SpelltowerBoard::MightHaveWords(
    const std::vector<std::string> &words) const {
  std::vector<std::string> filtered_words;
  for (const auto &wd : words) {
    if (MightHaveWord(wd)) filtered_words.push_back(wd);
  }
  return filtered_words;
}

std::vector<std::string> SpelltowerBoard::MightHaveAllStarWords(
    const std::vector<std::string> &words) const {
  std::vector<std::string> filtered_words;
  for (const auto &wd : words) {
    if (MightHaveAllStarWord(wd)) {
      filtered_words.push_back(wd);
      LOG(INFO) << "Words found: " << filtered_words.size() << " "
                << absl::StrJoin(filtered_words, ", ");
      if (filtered_words.size() >= 10) break;
    }
  }
  return filtered_words;
}

void SpelltowerBoard::DFS(absl::string_view word, int i,
                          std::vector<LetterCount> &row_letter_counts,
                          std::vector<Point> &path,
                          std::vector<std::vector<Point>> &paths) const {
  if (i >= word.length()) {
    for (const Point &star : stars_) {
      if (!absl::c_contains(path, star)) return;
    }
    paths.push_back(path);
    return;
  }

  char c = word[i];
  for (const Point &p : letter_map_.at(c)) {
    if ((!path.empty() && std::abs(p.row - path.back().row) > 1) ||
        !row_letter_counts[p.row].contains(c))
      continue;
    path.push_back(p);
    if (auto e = row_letter_counts[p.row].RemoveLetter(c); !e.ok()) {
      LOG(ERROR) << e.status();
      return;
    }

    DFS(word, i + 1, row_letter_counts, path, paths);

    path.pop_back();
    if (auto e = row_letter_counts[p.row].AddLetter(c); !e.ok()) {
      LOG(ERROR) << e.status();
      return;
    }
  }
}

// - If we've reached the end of the word, return true.
// - Check row `r` for `word[i]`. If we don't find it, return false.
// - If we find it, then we need to look for the next letter in the next set of
//   possible rows. If any of these return true, return true.
bool SpelltowerBoard::DFS(absl::string_view word, int i, int row) const {
  if (i >= word.length()) return true;
  if (row < 0 || row >= rows_) return false;
  if (!absl::c_contains(board_[row], word[i])) return false;

  return (DFS(word, i + 1, row - 1) || DFS(word, i + 1, row) ||
          DFS(word, i + 1, row + 1));
}

bool SpelltowerBoard::DFS(absl::string_view word, int i, int row,
                          std::vector<Point> &path,
                          std::vector<bool> &used_stars,
                          std::vector<LetterCount> &row_letter_counts) const {
  // If we've found the whole word, return true!
  if (i >= word.length()) {
    return std::all_of(used_stars.begin(), used_stars.end(),
                       [](bool b) { return b; });
  }

  if (row < 0 || rows_ <= row || !row_letter_counts[row].contains(word[i]) ||
      !absl::c_contains(board_[row], word[i]))
    return false;

  int j = 0;
  bool star_matched = false;
  for (; j < stars_.size(); ++j) {
    if (used_stars[j] || stars_[j].row != row) continue;
    if (word[i] == char_at(stars_[j])) {
      star_matched = true;
      used_stars[j] = true;
      break;
    }
  }

  if (auto s = row_letter_counts[row].RemoveLetter(word[i]); !s.ok()) {
    return false;
  }

  bool ret = (DFS(word, i + 1, row - 1, path, used_stars, row_letter_counts) ||
              DFS(word, i + 1, row, path, used_stars, row_letter_counts) ||
              DFS(word, i + 1, row + 1, path, used_stars, row_letter_counts));

  if (auto s = row_letter_counts[row].AddLetter(word[i]); !s.ok()) {
    LOG(ERROR) << s;
    return false;
  }
  if (star_matched) {
    used_stars[j] = false;
  }
  return ret;
}

}  // namespace puzzmo