#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "spelltower.h"

namespace puzzmo {

void Spelltower::DFS(std::vector<std::vector<bool>> &visited, Point p,
                     std::shared_ptr<TrieNode> node,
                     absl::flat_hash_set<Point> &path, WordMap &ans) {
  if (!Valid(p) || visited[p.row][p.col] || LetterAt(p) == '*' ||
      LetterAt(p) == ' ') {
    return;
  }

  const char c = LetterAt(p);
  std::shared_ptr<TrieNode> child = node->children[c - 'a'];
  if (child == nullptr) {
    return;
  }

  path.insert(p);
  visited[p.row][p.col] = true;

  if (child->word != nullptr) {
    int s = Score(path);
    ans[s].insert(*child->word);
  }

  for (const auto &dp : kAdjacentCoords) {
    DFS(visited, p + dp, child, path, ans);
  }

  path.erase(p);
  visited[p.row][p.col] = false;
}

const WordMap Spelltower::FindWords() {
  absl::flat_hash_set<Point> path;
  WordMap words;
  std::vector<std::vector<bool>> visited(n_, std::vector<bool>(m_));
  for (int i = 0; i < n_; ++i) {
    for (int j = 0; j < m_; ++j) {
      DFS(visited, {.row = i, .col = j}, dict_, path, words);
    }
  }
  return words;
}

const char Spelltower::LetterAt(Point p) { return board_[p.row][p.col]; }

int Spelltower::Score(const absl::flat_hash_set<Point> &path) {
  absl::flat_hash_set<Point> affected;
  for (const Point p : path) {
    affected.insert(p);
    char c = LetterAt(p);
    if (c == 'j' || c == 'q' || c == 'x' || c == 'z') {
      for (int j = 0; j < m_; ++j) {
        affected.insert({.row = p.row, .col = j});
      }
    }
    if (path.size() < 5) {
      continue;
    }

    for (const Point dp : kDPad) {
      if (!Valid(p + dp))
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

bool Spelltower::Valid(const Point p) {
  return Spelltower::Valid(p.row, p.col);
}

bool Spelltower::Valid(int r, int c) {
  return (r >= 0 && r < n_ && c >= 0 && c < m_);
}

} // namespace puzzmo