#include "spelltower_solver.h"

#include <memory>
#include <vector>

#include "absl/log/log.h"
#include "re2/re2.h"

namespace puzzmo {

WordMap SpelltowerSolver::AllWordsOnBoard(const SpelltowerBoard &board) const {
  SpelltowerPath path;
  WordMap words;
  for (const auto &p : board.points()) {
    if (!std::isalpha(board[p])) continue;
    path.push_back(p);
    LOG(INFO) << p << " \"" << board[p] << "\"";
    DFSAllWordsOnBoard(p, dict_->children[board[p] - 'a'], board, path, words);
    path.pop_back();
  }
  return words;
}

absl::StatusOr<SpelltowerPath> SpelltowerSolver::LongestPossibleAllStarWord()
    const {
  auto maybe_star_words = ReadDictionaryFileToVector(
      {.min_letters = 15,
       .min_letter_count = LetterCount(board_.StarLetters())});
  if (!maybe_star_words.ok()) return maybe_star_words.status();

  // std::sort(maybe_star_words->begin(), maybe_star_words->end(),
  //           [](std::string a, std::string b) {
  //             if (a.length() == b.length()) return a < b;
  //             return a.length() < b.length();
  //           });
  std::vector<std::string> filtered_words =
      MightHaveAllStarWords(*maybe_star_words);

  std::string rgx = absl::StrJoin(board_.GetAllStarRegexes(), "|",
                                  [](std::string *out, const std::string &in) {
                                    absl::StrAppend(out, "(", in, ")");
                                  });
  for (const auto &w : filtered_words) {
    if (RE2::PartialMatch(w, rgx)) LOG(INFO) << w;
  }
  SpelltowerPath path;
  return path;
}

void SpelltowerSolver::DFSAllWordsOnBoard(
    const Point &loc, const std::shared_ptr<TrieNode> &trie_node,
    const SpelltowerBoard &board, SpelltowerPath &path, WordMap &ans) const {
  std::string wd = absl::StrJoin(
      path.points(), "", [board](std::string *out, const Point &pt) {
        absl::StrAppend(out, std::string(1, board[pt]));
      });

  // If we currently have a word, add it to our set of answers.
  if (trie_node->word != nullptr) {
    int s = board.ScorePath(path);

    LOG(INFO) << std::string(path.size(), '|') << " Word found! \""
              << *(trie_node->word) << "\", scoring " << s;
    ans[s].insert(*trie_node->word);
  }

  // For every possible choice in the current position...
  for (const Point &p : board.ValidMooreNeighbors(loc)) {
    if (path.contains(p)) continue;

    std::shared_ptr<TrieNode> child = trie_node->children[board[p] - 'a'];

    if (child == nullptr) {
      LOG(INFO) << std::string(path.size(), '|')
                << absl::StrFormat(" %v \"%s%c\" %s", p, wd, board[p],
                                   "is not a word prefix.");
      continue;
    }

    // ...make it...
    path.push_back(p);

    LOG(INFO) << std::string(path.size() - 1, '|')
              << absl::StrFormat("+ %v + \'%c\' = \"%s\"", p, board[p],
                                 wd + board[p]);

    // ...recurse...
    DFSAllWordsOnBoard(p, child, board, path, ans);

    // ...and backtrack!
    path.pop_back();
  }
  LOG(INFO) << std::string(path.size() - 1, '|');
}

std::vector<std::string> SpelltowerSolver::MightHaveAllStarWords(
    const std::vector<std::string> &words) const {
  std::vector<std::string> filtered_words;
  for (const auto &wd : words) {
    if (MightHaveAllStarWord(wd)) {
      filtered_words.push_back(wd);
    }
  }
  return filtered_words;
}

bool SpelltowerSolver::MightHaveAllStarWord(absl::string_view word) const {
  std::vector<LetterCount> row_letter_counts = board_.GetRowLetterCounts();
  SpelltowerPath path;
  if (DFSHighScoringWord(board_, word, 0, row_letter_counts, path)) {
    return true;
  }
  return false;
}

bool SpelltowerSolver::DFSHighScoringWord(
    const SpelltowerBoard &board, absl::string_view word, int i,
    std::vector<LetterCount> &row_letter_counts, SpelltowerPath &path) const {
  // std::cout << "\33[2K" << word.substr(0, i) << "\r";
  // Check if we have a solution
  if (i >= word.length()) {
    if (path.num_stars() == board.NumStars()) return false;
    return IsPathPossible(path);
  }

  // For every possible choice in the current position...
  const char c = word[i];
  auto letter_map = board.letter_map();
  for (const Point &p : letter_map[c]) {
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
    if (DFSHighScoringWord(board, word, i + 1, row_letter_counts, path))
      return true;

    // Unmake the choice
    path.pop_back();
    if (auto e = row_letter_counts[p.row].AddLetter(c); !e.ok()) {
      LOG(ERROR) << e.status();
      return false;
    }
  }
  return false;
}

bool SpelltowerSolver::IsPathPossible(SpelltowerPath &path) const {
  // Check for interrupted columns (or "A-C-B columns")
  for (int i = 1; i < path.size(); ++i) {
    if (path[i - 1].row == path[i].row &&
        std::abs(path.num_below(i - 1) - path.num_below(i)) > 1)
      return false;
  }

  bool aligned = false;
  while (!aligned) {
    // Start with the lowest point
    std::vector<int> l_to_h = path.IndicesByColumn();
    for (int i = 0; i < l_to_h.size(); ++i) {
      int idx = l_to_h[i];
      Point &curr = path[idx];
      if (idx > 0) {
        Point &prev = path[idx - 1];
        if (!curr.MooreNeighbors().contains(prev)) {
          if (!UpdatePath(path, idx - 1)) return false;
          break;  // Restarts the for loop.
        }
      }
      if (idx < path.size() - 1) {
        Point &next = path[idx + 1];
        if (!curr.MooreNeighbors().contains(next)) {
          if (!UpdatePath(path, idx)) return false;
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

bool SpelltowerSolver::UpdatePath(SpelltowerPath &path, int l) const {
  int lo = (path[l].col < path[l + 1].col) ? l : l + 1;
  int hi = (path[l + 1].col < path[l].col) ? l : l + 1;
  if (path[lo].col + 1 < path.num_below(hi)) return false;

  // Lower hi to one column above lo. Lowers any points above hi as well.
  int shift = path[hi].col - path[lo].col - 1;
  std::vector<int> row_items = path.SimplifiedRow(path[hi].row);
  bool above_hi = true;
  for (int i = row_items.size() - 1; i >= 0; --i) {
    int idx = row_items[i];
    // Anything above the point being lowered needs to drop the same amount.
    if (above_hi) {
      path[idx].col -= shift;
      if (idx == hi) above_hi = false;
      continue;
    }
    // If we're below the point being lowered, we lower it as little as needed
    // to stay below the point above it.
    // It's safe to refer to row_items[i+1].
    path[idx].col = path[row_items[i + 1]].col - 1;
  }
  return true;
}

}  // namespace puzzmo