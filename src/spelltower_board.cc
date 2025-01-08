#include "spelltower_board.h"

namespace puzzmo {

const char SpelltowerBoard::LetterAt(Point p) { return board_[p.row][p.col]; }

bool SpelltowerBoard::HasPoint(const Point p) {
  return SpelltowerBoard::HasPoint(p.row, p.col);
}

bool SpelltowerBoard::HasPoint(int row, int col) {
  return (row >= 0 && row < rows_ && col >= 0 && col < cols_);
}

int SpelltowerBoard::Score(const absl::flat_hash_set<Point> &path) {
  absl::flat_hash_set<Point> affected;
  for (const Point p : path) {
    affected.insert(p);
    char c = LetterAt(p);
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
    char c = LetterAt(p);
    if (c == ' ' || c == '*')
      continue;
    score += kLetterScores[c - 'a'];
  }
  score *= path.size();
  return score;
}

} // namespace puzzmo