// -----------------------------------------------------------------------------
// File: solver.h
// -----------------------------------------------------------------------------
//
// This header file defines the solver class for Spelltower. It contains a
// dictionary, helper methods to interact with the board using the dictionary,
// and various solution methods.

#ifndef PUZZMO_SPELLTOWER_SOLVER_H_
#define PUZZMO_SPELLTOWER_SOLVER_H_

#include <string>
#include <vector>

#include "absl/container/btree_map.h"
#include "absl/container/btree_set.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "dict.h"
#include "grid.h"
#include "path.h"
#include "src/shared/letter_count.h"

namespace puzzmo::spelltower {

// spelltower::Solver
//
// The `Solver` class is used to output possible solutions to a Spelltower
// puzzle. It is the highest-level class in the library.
class Solver {
 public:
  //--------------
  // Constructors

  // The simplest constructor for a `Solver`, taking a `Dict` and a `Grid`. For
  // simplicity, the grid can be provided in the form of `grid_strings`.
  Solver(const Dict& dict, const Grid& grid)
      : dict_(dict), grid_(grid), word_score_sum_(0) {}
  Solver(const Dict& dict, const std::vector<std::string>& grid_strings)
      : Solver(dict, Grid(grid_strings)) {}

  // The dict can also be created from a trie, although this is less efficient.
  Solver(const Trie& trie, const Grid& grid)
      : dict_(trie), grid_(grid), word_score_sum_(0) {}
  Solver(const Trie& trie, const std::vector<std::string>& grid_strings)
      : Solver(trie, Grid(grid_strings)) {}

  // A static method that creates a `Solver` with only a `Grid`, loading the
  // `Dict` from a serialized string. Creating the dict in this way can be done
  // in a single DFS trie traversal, which is less expensive.
  static absl::StatusOr<Solver> CreateSolverWithSerializedDict(
      const Grid& grid);
  static absl::StatusOr<Solver> CreateSolverWithSerializedDict(
      const std::vector<std::string>& grid);

  //-----------
  // Accessors

  // Solver::dict()
  //
  // Provides access to the underlying `Dict`.
  const Dict dict() const { return dict_; }

  // Solver::grid()
  //
  // Provides access to the `Grid` to which the `Solver` has been making
  // changes.
  Grid grid() const { return grid_; }

  // Solver::TileAt()
  //
  // Syntactic sugar for accessing grid tiles directly.
  std::shared_ptr<Tile> TileAt(const Point& p) const {
    return TileAt(p.row, p.col);
  }
  std::shared_ptr<Tile> TileAt(int row, int col) const {
    return grid_[{.row = row, .col = col}];
  }

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
  // objects that have been played in sequence to transform `grid_` to its
  // current state.
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
  // Mutators

  // Solver::reset()
  //
  // Returns the solver to its starting state.
  absl::Status reset();

  // Solver::PlayWord()
  //
  // Removes all tiles affected by `word` from `grid_`, adds the score to
  // `words_score_`, and updates `solution_` and `snapshots_`. Clears
  // `word_cache`.
  //
  // Fails if `word` is empty, if `word` is non-continuous, or if `word.word()`
  // is not in `trie_`.
  absl::Status PlayWord(const Path& word);

  // Solver::UndoLastPlay()
  //
  // Reverts all changes as if the last `PlayWord()` had never been called.
  // Reverts `grid_`, subtracts the score from `words_score_`, and removes the
  // most recent entries from `solution_` and `snapshots_`. Clears
  // `word_cache_`.
  absl::Status UndoLastPlay();

  //------------------
  // Solution methods

  // Solver::SolveGreedily()
  //
  // Repeatedly plays the highest-scoring word available until no more words can
  // be found.
  absl::Status SolveGreedily();

  // Solver::SolveWithOneLongWord()
  //
  // Plays a sequence of short words to align tiles, then one long,
  // high-multiplier word. After playing the long word, solves greedily.
  absl::Status SolveWithOneLongWord();

  //----------
  // Helpers

  // Solver::BestPossibleGoalWord()
  //
  // Gets the letter count and both 2* and 3* regexes from `grid_`, then
  // searches `dict_` for them. Starting from the longest word returned, calls
  // `BestPossibleTwoStarPathForWord()` on the word. When we have a hit, if it's
  // a 3* word, returns the path immediately; if it's a 2* word, continues the
  // search using only 3* words and requiring three-star paths until we reach a
  // short enough length that a three-star word would no longer have a higher
  // multiplier.
  //
  // Example: If we find a 22-long word using only 2 stars, the multiplier will
  // be x66. After we finish searching the length-22 words, we loop length-21
  // words that use all 3 stars, then 20, down until we finish length-17 (x68)
  absl::StatusOr<Path> BestPossibleGoalWord() const;

  // A TEMPORARY method that calls `StepsToPlayGoalWordDFS()`. Will be removed
  // once `SolveWithOneLongWord()` is assembled.
  absl::Status PlayGoalWord(const Path& goal_word);

  // Solver::BestPossiblePathForWord()
  //
  // Constructs and returns the best possible path for a given word. This path
  // is not necessarily continuous, and is likely not. "Best" is determined by
  // which path has the highest multiplier. In the event of a tie, the path with
  // the lowest delta is chosen.
  absl::StatusOr<Path> BestPossiblePathForWord(absl::string_view word) const;

  // Solver::BestPossibleTwoStarPathForWord()
  //
  // Functions identically to `BestPossiblePathForWord()`, but necessitates that
  // the path contain at least two of the star tiles on the grid.
  absl::StatusOr<Path> BestPossibleTwoStarPathForWord(
      absl::string_view word) const;

  // Solver::BestPossibleThreeStarPathForWord()
  //
  // Functions identically to `BestPossiblePathForWord()`, but necessitates that
  // the path contain all three star tiles on the grid. As a result, "best" will
  // always come down to lowest delta.
  absl::StatusOr<Path> BestPossibleThreeStarPathForWord(
      absl::string_view word) const;

  // Solver::FillWordCache()
  //
  // If `cache` is empty, runs DFS on the grid and populates `cache` with the
  // results. Does not run if `cache` is populated. If called without a
  // parameter, fills `word_cache_`.
  void FillWordCache() { FillWordCache(word_cache_); };
  void FillWordCache(
      absl::btree_map<int, absl::btree_set<Path>, std::greater<int>>& cache);

 private:
  // Solver::BestPathDFS()
  //
  // A recursive helper method called by `BestPossiblePathForWord()`.
  void BestPathDFS(absl::string_view word, int i, Path& path,
                   Path& best_path) const;

  // Solver::TwoStarDFS()
  //
  // A recursive helper method called by `BestPossibleTwoStarPathForWord()`.
  // Works the same as `BestPossiblePathForWord()`, with these changes:
  // - If a star tile is added to `path`, that letter is removed from
  //   `unused_star_letters` before recursing, and added back after.
  // - If at any point the rest of the word does not contain two of the
  //   `unused_star_letters`, cuts the branch short immediately.
  // - If the path contains the whole word but doesn't have at least two stars
  //   in it, does not consider it for `best_path`.
  void TwoStarDFS(absl::string_view word, int i,
                  LetterCount& unused_star_letters, Path& path,
                  Path& best_path) const;

  // Solver::ThreeStarDFS()
  //
  // A recursive helper method called by `BestPossibleThreeStarPathForWord()`.
  // Works the same as `BestPossiblePathForWord()`, with these changes:
  // - If a star tile is added to `path`, that letter is removed from
  //   `unused_star_letters` before recursing, and added back after.
  // - If at any point the rest of the word does not contain one of the
  //   `unused_star_letters`, cuts the branch short immediately.
  // - If the path contains the whole word but doesn't have all stars in it,
  //   does not consider it for `best_path`.
  void ThreeStarDFS(absl::string_view word, int i,
                    LetterCount& unused_star_letters, Path& path,
                    Path& best_path) const;

  // Solver::CacheDFS()
  //
  // A recursive helper method called by `FillWordCache()`. In parallel,
  // searches `trie_` and `grid_` depth-first from the node and the last tile in
  // `path`.
  void CacheDFS(
      const std::shared_ptr<TrieNode>& trie_node, Path& path,
      absl::btree_map<int, absl::btree_set<Path>, std::greater<int>>& cache);

  // Solver::StepsToPlayGoalWordDFS()
  //
  // A recursive helper method called by `TwoStarDFS()` and `ThreeStarDFS()`.
  // Given `path`, recursively tries to play words in order to make the path
  // continuous. If a sequence of words that does so is found, returns the
  // vector of paths that do so. If not, returns an error.
  //
  // While the method is not const, this should not change the internal state of
  // the solver object.
  absl::StatusOr<std::vector<Path>> StepsToPlayGoalWordDFS(
      const Path& goal_word);

  const Dict dict_;
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

#endif