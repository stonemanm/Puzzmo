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
#include "src/shared/letter_count.h"
#include "trie.h"

namespace puzzmo::spelltower {

// spelltower::Solver
//
// The `Solver` class is used to output possible solutions to a Spelltower
// puzzle. It is the highest-level class in the library.
class Solver {
 public:
  //--------------
  // Constructors

  // The simplest constructor for a `Solver`, taking a `Trie` and a `Grid`. For
  // simplicity, the grid can be provided in the form of `grid_strings`.
  Solver(const Trie& trie, const Grid& grid)
      : trie_(trie), starting_grid_(grid), grid_(grid), word_score_sum_(0) {}
  Solver(const Trie& trie, const std::vector<std::string>& grid_strings)
      : Solver(trie, Grid(grid_strings)) {}

  // A static method that creates a `Solver` with only a `Grid`, loading the
  // `Trie` from a serialized string. Creating the trie in this way can be done
  // in a single DFS traversal, which is less expensive.
  static absl::StatusOr<Solver> CreateSolverWithSerializedTrie(
      const Grid& grid);
  static absl::StatusOr<Solver> CreateSolverWithSerializedTrie(
      const std::vector<std::string>& grid);

  //-----------
  // Accessors

  // Solver::trie()
  //
  // Provides access to the underlying `Trie`.
  const Trie trie() const { return trie_; }

  // Solver::starting_grid()
  //
  // Provides access to the `Grid` with which the `Solver` was created.
  const Grid starting_grid() const { return starting_grid_; }

  // Solver::grid()
  //
  // Provides access to the `Grid` to which the `Solver` has been making
  // changes.
  Grid grid() const { return grid_; }

  // Solver::word_cache()
  //
  // Returns a data structure containing all the words currently on `grid_`.
  absl::btree_map<int, absl::btree_set<Path>, std::greater<int>> word_cache()
      const {
    return word_cache_;
  }

  // Solver::solution()
  //
  // Provides access to the solution thus far, which is the vector of `Path`
  // objects that have been played in sequence to transform `starting_grid_` to
  // `grid_`.
  std::vector<Path> solution() const { return solution_; }

  // Solver::snapshots()
  //
  // Returns the vector of snapshots, which provides a visualization of the
  // solution. For every entry in `solution_`, `snapshots_` contains a string
  // representation of the footprint of that path on the grid.
  std::vector<std::string> snapshots() const { return snapshots_; }

  // Solver::score()
  //
  // Returns the total score that will be obtained by playing the current
  // solution. This includes the score from every word played as well as the
  // score bonuses for clearing most or all of the grid.
  int score() const { return word_score_sum_ + grid_.ScoreBonuses(); }

  // Solver::AlmostThere()
  //
  // Returns `true` if every column in `grid_` has at most two tiles in it.
  bool AlmostThere() const { return grid_.AlmostThere(); }

  // Solver::FullClear()
  //
  // Returns `true` if `grid_` no longer has tiles in it.
  bool FullClear() const { return grid_.FullClear(); }

  //----------
  // Helpers

  // Solver::BestPossiblePathForWord()
  //
  // Constructs and returns the best possible path for a given word. This path
  // is not necessarily continuous, and is likely not. "Best" is determined by
  // which path returns the highest score. In the event of a tie, the path with
  // the lowest delta is chosen.
  absl::StatusOr<Path> BestPossiblePathForWord(absl::string_view word) const;

  // Solver::BestPossibleAllStarPathForWord()
  //
  // Functions identically to `BestPossiblePathForWord()`, but with the added
  // constraint that the resulting path must contain every star tile in the
  // grid.
  absl::StatusOr<Path> BestPossibleAllStarPathForWord(
      absl::string_view word) const;

  // Solver::FillWordCache()
  //
  // If `word_cache_` is empty, runs DFS on the grid and populates `word_cache_`
  // with the results. Does not run if `word_cache_` is populated.
  void FillWordCache();

  //----------
  // Mutators

  // Solver::reset()
  //
  // Returns the solver to its starting state.
  void reset();

  // Solver::PlayWord()
  //
  // Removes all tiles affected by `word` from `grid_`, adds the score to
  // `words_score_`, and updates `solution_` and `snapshots_`. Clears
  // `word_cache`.
  //
  // Fails if `word` is empty, if `word` is non-continuous, or if `word.word()`
  // is not in `trie_`.
  absl::Status PlayWord(const Path& word);

  //------------------
  // Solution methods

  // Solver::SolveGreedily()
  //
  // Repeatedly plays the highest-scoring word available until no more words can
  // be found.
  absl::Status SolveGreedily();

 private:
  // Solver::BestPossiblePathForWordDFS()
  //
  // A recursive helper method called by `BestPossiblePathForWord()`.
  void BestPossiblePathForWordDFS(absl::string_view word, int i, Path& path,
                                  Path& best_path) const;

  // Solver::BestPossibleAllStarPathForWordDFS()
  //
  // A recursive helper method called by `BestPossibleAllStarPathForWord()`.
  // Works the same as `BestPossiblePathForWord()`, with these changes:
  // - If a star tile is added to `path`, that letter is removed from
  //   `unused_star_letters` before recursing, and added back after.
  // - If at any point the rest of the word does not contain one of the
  //   `unused_star_letters`, cuts the branch short immediately.
  // - If the path contains the whole word but doesn't have all stars in it,
  //   does not consider it for `best_path`.
  void BestPossibleAllStarPathForWordDFS(absl::string_view word, int i,
                                         LetterCount& unused_star_letters,
                                         Path& path, Path& best_path) const;

  // Solver::FillWordCcheDFS()
  //
  // A recursive helper method called by `FillWordCache()`. In parallel,
  // searches `trie_` and `grid_` depth-first from the node and the last tile in
  // `path`.
  void FillWordCacheDFS(const std::shared_ptr<TrieNode>& trie_node, Path& path);

  const Trie trie_;
  const Grid starting_grid_;
  Grid grid_;
  absl::btree_map<int, absl::btree_set<Path>, std::greater<int>> word_cache_;
  std::vector<Path> solution_;
  std::vector<std::string> snapshots_;
  int word_score_sum_;

  //------------------
  // Abseil functions

  template <typename Sink>
  friend void AbslStringify(Sink& sink, const Solver& solver) {
    for (int i = 0; i < solver.solution().size(); ++i) {
      sink.Append(absl::StrCat(i + 1, ". \"", solver.solution()[i].word(),
                               "\"\n", solver.snapshots_[i], "\n\n"));
    }
    absl::Format(&sink, "%v", solver.grid_);
  }
};

}  // namespace puzzmo::spelltower

#endif  // !solver_h