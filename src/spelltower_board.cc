#include "spelltower_board.h"

#include <cctype>

namespace puzzmo {

SpelltowerBoard::SpelltowerBoard(const std::vector<std::vector<char>> &board)
    : board_(board), rows_(board.size()) {
  int max_row_size = 0;
  for (const auto &row : board_) {
    if (max_row_size >= row.size())
      continue;
    max_row_size = row.size();
  }
  cols_ = max_row_size;
  for (auto &row : board_) {
    while (row.size() < cols_) {
      row.push_back(' ');
    }
  }
};

char SpelltowerBoard::LetterAt(const Point &p) const {
  return board_[p.row][p.col];
}

bool SpelltowerBoard::HasPoint(const Point &p) const {
  return SpelltowerBoard::HasPoint(p.row, p.col);
}

bool SpelltowerBoard::HasPoint(int row, int col) const {
  return (row >= 0 && row < rows_ && col >= 0 && col < cols_);
}

int SpelltowerBoard::NumRows() const { return rows_; }
int SpelltowerBoard::NumCols() const { return cols_; }

int SpelltowerBoard::Score(const absl::flat_hash_set<Point> &path) const {
  absl::flat_hash_set<Point> affected;
  int star_tiles = 0;
  for (const Point p : path) {
    affected.insert(p);
    char c = LetterAt(p);
    if (std::isupper(c)) {
      ++star_tiles;
      c = std::tolower(c);
    }
    if (c == 'j' || c == 'q' || c == 'x' || c == 'z') {
      for (int col = 0; col < cols_; ++col) {
        affected.insert({.row = p.row, .col = col});
      }
    }
    if (path.size() < 5)
      continue;

    for (const Point dp : kDPad) {
      if (!HasPoint(p + dp))
        continue;
      affected.insert(p + dp);
    }
  }

  int score = 0;
  for (const Point p : affected) {
    char c = std::tolower(LetterAt(p));
    if (c == ' ' || c == '*')
      continue;
    score += kLetterScores[c - 'a'];
  }
  score *= path.size();
  return score *= (1 + star_tiles);
}

} // namespace puzzmo