#ifndef spelltower_solver_h
#define spelltower_solver_h

#include <functional>
#include <memory>
#include <string>

#include "absl/container/btree_map.h"
#include "absl/container/btree_set.h"
#include "spelltower_board.h"
#include "src/shared/dictionary_utils.h"
#include "src/shared/point.h"
#include "src/spelltower/spelltower_path.h"

namespace puzzmo::spelltower {
using WordMap =
    absl::btree_map<int, absl::btree_set<std::string>, std::less<int>>;

class SpelltowerSolver {
 public:
  SpelltowerSolver(SpelltowerBoard &board, const std::shared_ptr<TrieNode> dict)
      : board_(board), starting_board_(board), dict_(dict) {};

  // Returns a map of all valid words on the board, as well as their score
  WordMap AllWordsOnBoard(const SpelltowerBoard &board) const;

  absl::StatusOr<SpelltowerPath> LongestPossibleAllStarWord() const;

  std::vector<std::string> MightHaveAllStarWords(
      const std::vector<std::string> &words) const;

  bool MightHaveAllStarWord(absl::string_view word) const;

  bool IsPathPossible(SpelltowerPath &path) const;

 private:
  void DFSAllWordsOnBoard(const Point &loc,
                          const std::shared_ptr<TrieNode> &trie_node,
                          const SpelltowerBoard &board, SpelltowerPath &path,
                          WordMap &ans) const;

  bool DFSHighScoringWord(const SpelltowerBoard &board, absl::string_view word,
                          int i, std::vector<LetterCount> &row_letter_counts,
                          SpelltowerPath &path) const;

  bool UpdatePath(SpelltowerPath &path, int l) const;

  SpelltowerBoard board_;
  const SpelltowerBoard starting_board_;
  const std::shared_ptr<TrieNode> dict_;
};

}  // namespace puzzmo::spelltower

#endif
