#include "spelltower_board.h"

#include <cctype>

namespace puzzmo {

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
        stars_.insert({r, c});
      }
    }
  }
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

absl::flat_hash_set<Point> SpelltowerBoard::StarLocations() const {
  return stars_;
}

int SpelltowerBoard::Score(const absl::flat_hash_set<Point> &path) const {
  absl::flat_hash_set<Point> affected;
  int star_tiles = 0;
  for (const Point p : path) {
    affected.insert(p);
    if (stars_.contains(p))
      ++star_tiles;

    char c = std::tolower(At(p));
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
    char c = std::tolower(At(p));
    if (c == ' ' || c == '*')
      continue;
    score += kLetterScores[c - 'a'];
  }
  score *= path.size();
  return score *= (1 + star_tiles);
}

bool SpelltowerBoard::MightHaveWord(const std::string &word) const {
  for (int row = 0; row < rows_; ++row) {
    if (DFS(word, 0, row))
      return true;
  }
  return false;
}

std::vector<std::string>
SpelltowerBoard::MightHaveWords(const std::vector<std::string> &words) const {
  std::vector<std::string> filtered_words;
  for (const auto &wd : words) {
    if (MightHaveWord(wd))
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

} // namespace puzzmo