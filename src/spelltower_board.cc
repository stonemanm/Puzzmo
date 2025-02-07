#include "spelltower_board.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>

#include "absl/log/log.h"
#include "absl/strings/str_cat.h"

namespace puzzmo {
namespace {

int EmptyRowsBetween(const Point &p1, const Point &p2) {
  int dist = std::abs(p1.row - p2.row);
  return dist == 0 ? 0 : dist - 1;
}

} // namespace

SpelltowerBoard::SpelltowerBoard(const std::vector<std::vector<char>> &board)
    : board_(board), rows_(board.size()) {
  cols_ = 0;
  for (const auto &row : board_) {
    if (cols_ < row.size())
      cols_ = row.size();
  }
  for (auto &row : board_) {
    while (row.size() < cols_) {
      row.push_back(' ');
    }
  }

  for (int r = 0; r < rows_; ++r) {
    for (int c = 0; c < cols_; ++c) {
      char ltr = At(r, c);
      if (std::isupper(ltr)) {
        stars_.push_back({r, c});
        board_[r][c] = std::tolower(ltr);
      }
    }
  }
  std::sort(stars_.begin(), stars_.end());
};

char SpelltowerBoard::At(const Point &p) const {
  return SpelltowerBoard::At(p.row, p.col);
}

char SpelltowerBoard::At(int row, int col) const {
  if (!HasPoint(row, col))
    return '*';
  return board_[row][col];
}

bool SpelltowerBoard::HasPoint(const Point &p) const {
  return SpelltowerBoard::HasPoint(p.row, p.col);
}

bool SpelltowerBoard::HasPoint(int row, int col) const {
  return (row >= 0 && row < rows_ && col >= 0 && col < cols_);
}

absl::flat_hash_set<Point>
SpelltowerBoard::ValidVonNeumannNeighbors(const Point &p) const {
  absl::flat_hash_set<Point> neighbors = p.VonNeumannNeighbors();
  absl::erase_if(neighbors, [this](Point n) { return !HasPoint(n); });
  return neighbors;
}

absl::flat_hash_set<Point>
SpelltowerBoard::ValidMooreNeighbors(const Point &p) const {
  absl::flat_hash_set<Point> neighbors = p.MooreNeighbors();
  absl::erase_if(neighbors, [this](Point n) { return !HasPoint(n); });
  return neighbors;
}

int SpelltowerBoard::NumRows() const { return rows_; }
int SpelltowerBoard::NumCols() const { return cols_; }
int SpelltowerBoard::NumStars() const { return stars_.size(); }

absl::flat_hash_set<std::string> SpelltowerBoard::GetAllStarRegexes() const {
  if (stars_.empty())
    return {};

  std::vector<Point> star_vec(stars_);
  absl::flat_hash_set<std::string> regex_set;
  do {
    std::string s(1, At(star_vec[0]));
    for (int i = 1; i < star_vec.size(); ++i) {
      int d = EmptyRowsBetween(star_vec[i - 1], star_vec[i]);
      if (d == 0) {
        absl::StrAppend(&s, ".*", std::string(1, At(star_vec[i])));
      } else {
        absl::StrAppend(&s, ".{", d, ",}", std::string(1, At(star_vec[i])));
      }
    }
    regex_set.insert(s);
  } while (std::next_permutation(star_vec.begin(), star_vec.end()));
  return regex_set;
}

std::vector<Point> SpelltowerBoard::StarLocations() const { return stars_; }

int SpelltowerBoard::Score(const absl::flat_hash_set<Point> &path) const {
  absl::flat_hash_set<Point> affected;
  int star_tiles = 0;
  for (const Point p : path) {
    affected.insert(p);
    if (absl::c_contains(stars_, p))
      ++star_tiles;

    char c = At(p);
    if (c == 'j' || c == 'q' || c == 'x' || c == 'z') {
      for (int row = 0; row < rows_; ++row) {
        affected.insert({.row = row, .col = p.col});
      }
    }
    if (path.size() < 5)
      continue;

    for (const Point n : ValidVonNeumannNeighbors(p)) {
      affected.insert(n);
    }
  }

  int score = 0;
  for (const Point p : affected) {
    char c = At(p);
    if (c == ' ' || c == '*')
      continue;
    score += kLetterScores[c - 'a'];
  }
  score *= path.size();
  return score *= (1 + star_tiles);
}

bool SpelltowerBoard::MightHaveWord(const std::string &word) const {
  return MightHaveWord(word, false);
}

bool SpelltowerBoard::MightHaveWord(const std::string &word,
                                    bool all_star) const {
  std::vector<Point> star_vec(stars_.begin(), stars_.end());

  for (int row = 0; row < rows_; ++row) {
    std::vector<bool> used_stars(star_vec.size(), false);
    if (DFS(word, 0, row, used_stars))
      return true;
  }
  return false;
}

std::vector<std::string>
SpelltowerBoard::MightHaveWords(const std::vector<std::string> &words) const {
  return MightHaveWords(words, false);
}

std::vector<std::string>
SpelltowerBoard::MightHaveWords(const std::vector<std::string> &words,
                                bool all_star) const {
  std::vector<std::string> filtered_words;
  for (const auto &wd : words) {
    if (MightHaveWord(wd, all_star))
      filtered_words.push_back(wd);
  }
  return filtered_words;
}

// - If we've reached the end of the word, return true.
// - Check row `r` for `word[i]`. If we don't find it, return false.
// - If we find it, then we need to look for the next letter in the next set of
//   possible rows. If any of these return true, return true.
bool SpelltowerBoard::DFS(const std::string &word, int i, int row) const {
  if (i >= word.length())
    return true;
  if (row < 0 || row >= rows_)
    return false;
  if (!absl::c_contains(board_[row], word[i]))
    return false;

  return (DFS(word, i + 1, row - 1) || DFS(word, i + 1, row) ||
          DFS(word, i + 1, row + 1));
}

bool SpelltowerBoard::DFS(const std::string &word, int i, int row,
                          std::vector<bool> &used_stars) const {
  if (i >= word.length() && std::all_of(used_stars.begin(), used_stars.end(),
                                        [](bool b) { return b; }))
    return true;
  if (row < 0 || row >= rows_)
    return false;
  if (!absl::c_contains(board_[row], word[i]))
    return false;

  int s = 0;
  for (; s < stars_.size(); ++s) {
    if (stars_[s].row != row)
      continue;
    if (word[i] == At(stars_[s])) {
      used_stars[s] = true;
      break;
    }
  }

  bool ret = (DFS(word, i + 1, row - 1, used_stars) ||
              DFS(word, i + 1, row, used_stars) ||
              DFS(word, i + 1, row + 1, used_stars));
  used_stars[s] = false;
  return ret;
}

} // namespace puzzmo