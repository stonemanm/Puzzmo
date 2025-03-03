// -----------------------------------------------------------------------------
// File: solver.h
// -----------------------------------------------------------------------------
//
// This header file defines the solver class for Spelltower. It contains a
// dictionary, helper methods to interact with the board using the dictionary,
// and various solution methods.

#ifndef solver_h
#define solver_h

#include <string>
#include <vector>

#include "absl/container/btree_map.h"
#include "absl/container/btree_set.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "grid.h"
#include "path.h"
#include "trie.h"

namespace puzzmo::spelltower {

class Solver {
 public:
  Solver(const Trie& trie, const Grid& grid)
      : trie_(trie), starting_grid_(grid), grid_(grid), words_score_(0) {}
  Solver(const Trie& trie, const std::vector<std::string>& grid)
      : Solver(trie, Grid(grid)) {}
  static absl::StatusOr<Solver> CreateSolverWithSerializedTrie(
      const Grid& grid);
  static absl::StatusOr<Solver> CreateSolverWithSerializedTrie(
      const std::vector<std::string>& grid);

  //-----------
  // Accessors

  const Trie trie() const { return trie_; }
  const Grid starting_grid() const { return starting_grid_; }
  Grid grid() const { return grid_; }
  absl::btree_map<int, absl::btree_set<Path>, std::greater<int>> word_cache()
      const {
    return word_cache_;
  }
  std::vector<Path> solution() const { return solution_; }
  int score() const { return words_score_ + grid_.ScoreBonuses(); }

  bool AlmostThere() const { return grid_.AlmostThere(); }
  bool FullClear() const { return grid_.FullClear(); }

  //----------
  // Helpers

  // Populates `word_cache_`.
  void FillWordCache();

  //----------
  // Mutators

  // Clears the solution.
  void reset();

  // Removes the word from `grid_`, adding the score to `words_score_`, then
  // pushes `word` into `solution_`. Clears `word_cache`.
  absl::Status PlayWord(const Path& word);

  // Repeatedly plays the highest-scoring word available until no more words can
  // be found.
  absl::Status SolveGreedily();

 private:
  // In parallel, searches `trie_` and `grid_` depth-first from the node and the
  // last tile in `path`.
  void FillWordCacheDFS(const std::shared_ptr<TrieNode>& trie_node, Path& path);

  const Trie trie_;
  const Grid starting_grid_;
  Grid grid_;
  absl::btree_map<int, absl::btree_set<Path>, std::greater<int>> word_cache_;
  std::vector<Path> solution_;
  int words_score_;

  //------------------
  // Abseil functions

  template <typename Sink>
  friend void AbslStringify(Sink& sink, const Solver& solver) {
    for (int i = 0; i < solver.solution().size(); ++i) {
      sink.Append(absl::StrCat(i + 1, ". ", solver.solution()[i], "\n\n"));
    }
    absl::Format(&sink, "%v", solver.grid_);
  }
};

}  // namespace puzzmo::spelltower

#endif  // !solver_h