#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "spelltower.h"

namespace puzzmo {

void Spelltower::AddToDictionary(const std::vector<std::string> words) {
  for (const auto &word : words) {
    if (word.length() < 3) {
      continue;
    }
    std::shared_ptr<TrieNode> node = root_;
    for (const char c : word) {
      const int i = c - 'a';
      if (node->children[i] == nullptr) {
        node->children[i] = std::make_shared<TrieNode>();
      }
      node = node->children[i];
    }
    node->word = &word;
  }
}

void Spelltower::DFS(std::vector<std::vector<bool>> &visited, Point p,
                     std::shared_ptr<TrieNode> node,
                     absl::flat_hash_set<Point> &path, WordMap &ans) {
  if (!Valid(p) || visited[p.i][p.j] || LetterAt(p) == '*' ||
      LetterAt(p) == ' ') {
    return;
  }

  const char c = LetterAt(p);
  std::shared_ptr<TrieNode> child = node->children[c - 'a'];
  if (child == nullptr) {
    return;
  }

  path.insert(p);
  visited[p.i][p.j] = true;

  if (child->word != nullptr) {
    int s = Score(path);
    ans[s].insert(*child->word);
  }

  for (const auto &dp : kAdjacentCoords) {
    DFS(visited, p + dp, child, path, ans);
  }

  path.erase(p);
  visited[p.i][p.j] = false;
}

const WordMap Spelltower::FindWords() {
  absl::flat_hash_set<Point> path;
  WordMap words;
  std::vector<std::vector<bool>> visited(n_, std::vector<bool>(m_));
  for (int i = 0; i < n_; ++i) {
    for (int j = 0; j < m_; ++j) {
      DFS(visited, {.i = i, .j = j}, root_, path, words);
    }
  }
  return words;
}

const char Spelltower::LetterAt(Point p) { return board_[p.i][p.j]; }

int Spelltower::Score(const absl::flat_hash_set<Point> &path) {
  absl::flat_hash_set<Point> affected;
  for (const Point p : path) {
    affected.insert(p);
    char c = LetterAt(p);
    if (c == 'j' || c == 'q' || c == 'x' || c == 'z') {
      for (int j = 0; j < m_; ++j) {
        affected.insert({.i = p.i, .j = j});
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

bool Spelltower::Valid(const Point p) { return Spelltower::Valid(p.i, p.j); }

bool Spelltower::Valid(int i, int j) {
  return (i >= 0 && i < n_ && j >= 0 && j < m_);
}

} // namespace puzzmo