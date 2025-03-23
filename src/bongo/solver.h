// -----------------------------------------------------------------------------
// File: solver.h
// -----------------------------------------------------------------------------
//
// This header file defines the solver class for Bongo. It contains a
// dictionary, helper methods to interact with the board using the dictionary,
// and various solution methods.

#ifndef PUZZMO_BONGO_SOLVER_H_
#define PUZZMO_BONGO_SOLVER_H_

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/types/span.h"
#include "dict.h"
#include "gamestate.h"

namespace puzzmo::bongo {

// bongo::Technique
//
// Each enumerated `Technique` corresponds to (a) one or more set of cells in
// the grid, (b) a `Solver::OptionsFor*()` class method in `bongo::Solver` that
// gathers the options to try, and (c) a `Solver::Fill*()` class method that
// places a given option on the given cells.
enum class Technique {
  // Technique::kFillMostRestrictedRow
  //
  // The default technique. Tries options for whatever row has the most
  // populated cells without having a word in it. Corresponds to
  // `Solver::OptionsForLine()` and `Solver::FillLine()`.
  kFillMostRestrictedRow = 0,

  // Technique::kFillBonusWord
  //
  // Try to fill the bonus line. Possible options are constrained by
  // `Parameters::tiles_for_bonus_words`. Corresponds to
  // `Solver::OptionsForBonusWord()` and `Solver::FillLine()`.
  kFillBonusWord,

  // Technique::kFillMultiplierTiles
  //
  // Try to fill the cells with multipliers. Possible options are constrained by
  // `Parameters::tiles_for_multiplier_tiles`. Corresponds to
  // `Solver::OptionsForMultiplierTiles()` and `Solver::FillMultiplierCells()`.
  kFillMultiplierTiles,
};

// bongo::Solver
//
// The `Solver` class is used to output possible solutions to a Bongo puzzle. It
// is the highest-level class in the library.
class Solver {
 public:
  struct Parameters {
    // The number to pass NMostValuableTiles, from which sets of 3 are chosen to
    // make possible bonus words. Note that increasing this n scales by O(n^2).
    int tiles_for_bonus_words = 7;

    // The number to pass NMostValuableTiles, from which sets are chosen to
    // place on the multiplier tiles. Note that increasing this n scales by
    // O(n^2).
    int tiles_for_multiplier_tiles = 4;
  };

  //--------------
  // Constructors

  // The simplest constructor for a `Solver`, taking a `Dict`, a `Gamestate`,
  // and some `Parameters`  For simplicity, the grid can be provided in the form
  // of `grid_strings`.
  Solver(const Dict &dict, const Gamestate &state, Parameters params);

  //-----------
  // Accessors

  // Solver::dict()
  //
  // Provides access to the underlying `Dict`.
  Dict dict() const { return dict_; }

  // Solver::starting_state()
  //
  // Returns the starting gamestate passed to the solver.
  Gamestate starting_state() const { return starting_state_; }

  // Solver::CurrentState()
  //
  // Returns the current gamestate managed by the solver.
  Gamestate CurrentState() const {
    return steps_.empty() ? starting_state_ : steps_.back();
  }

  // Solver::best_score()
  //
  // Returns the highest score of any `Gamestate` seen by the solver.
  int best_score() const { return best_score_; }

  // Solver::best_state()
  //
  // Returns the `Gamestate` that scored `best_score`.
  Gamestate best_state() const { return best_state_; }

  //---------
  // Solvers

  // Solver::Solve()
  //
  // Solves by first applying `kFillBonusWord`, then `kFillMultiplierTiles`, and
  // then repeating `kFillMostRestrictedRow`.
  absl::StatusOr<Gamestate> Solve();

  // Solver::SolveWithTechniquesInOrder()
  //
  // Applies the `techniques` in order, depth-first, ending each branch when
  // `IsComplete()` is `true`. If that point has not been reached once all
  // provided `techniques` have been applied, repeatedly applies
  // `Technique::kFillMostRestrictedRow` to finish the search. At the end of
  // every branch of the search, if the current gamestate scores higher than
  // `best_score_`, it is saved as `best_state_` and `best_score_` is updated.
  //
  // If no solutions are found, the process is repeated from the top with
  // `tiles_for_bonus_words_` and `tiles_for_multiplier_tiles_` both
  // incremented.
  absl::StatusOr<Gamestate> SolveWithTechniquesInOrder(
      absl::Span<const Technique> techniques);

  //----------
  // Mutators

  // Solver::reset()
  //
  // Returns the solver to its starting state.
  void reset() { steps_.clear(); }

 private:
  // Solver::RecursiveHelper()
  //
  // If the gamestate is complete, checks to see if we have a new best state.
  // Otherwise, applies the `i`th technique passed in, gathering the
  // corresponding options and calling the corresponding filler method, and then
  // calls itself with `i` incremented.
  //
  // If `i >= techniques.size()`, uses the technique
  // `Technique::kFillMostRestrictedRow`.
  absl::Status RecursiveHelper(absl::Span<const Technique> techniques, int i);

  // Solver::FillLine()
  //
  // Makes the relevant modification to `CurrentState()`, appending it to
  // `steps_` if successful.
  absl::Status FillLine(const std::vector<Point> &line, absl::string_view word);

  // Solver::FillMultiplierCells()
  //
  // Fills the empty multiplier cells one by one with letters from the front of
  // the string until either the cells or the letters run out. Multiplier cells
  // are filled from highest to lowest multiplier, with ties broken UtD, LtR.
  // Appends to `steps_` if successful.
  absl::Status FillMultiplierCells(const absl::string_view letters);

  // Solver::OptionsForBonusWord()
  //
  // Calculates and returns all words to be tried in the bonus word slot. In
  // addition to the available letters, options are limited by the dictionary,
  // the letters already placed in the bonus word, and by
  // `Parameters::tiles_for_bonus_words`.
  absl::flat_hash_set<std::string> OptionsForBonusWord() const;

  // Solver::OptionsForBonusWord()
  //
  // Calculates and returns all words to be tried in the line. In addition to
  // the available letters, options are limited by the dictionary and the
  // letters already placed in the line.
  absl::flat_hash_set<std::string> OptionsForLine(
      const std::vector<Point> &line) const;

  // Solver::OptionsForMultiplierTiles()
  //
  // Calculates and returns all words to be tried in the multiplier slot. In
  // addition to the available letters, options are limited by the dictionary,
  // any letters already placed in the multiplier slots, and by
  // `Parameters::tiles_for_multiplier_tiles`.
  absl::flat_hash_set<std::string> OptionsForMultiplierTiles() const;

  //---------
  // Scoring

  // Solver::LineScore()
  //
  // Gets the word from `line` and checks whether it is a valid word. If not,
  // returns 0. If it is, calculates the score from the letters and multipliers,
  // and multiplies by 1.3 if the word is a common word before returning.
  int LineScore(const std::vector<Point> &line) const;

  // Solver::Score()
  //
  // Sums `LineScore` for the six lines to be used in scoring.
  int Score() const;

  // Solver::UpdateBestState()
  //
  // Checks `CurrentState()` against `best_state_`, updating `best_state_` and
  // `best_score_` if the current state scores higher.
  void UpdateBestState();

  //-------
  // Words

  // Solver::GetWord()
  //
  // Returns the word that will be scored from the line in `CurrentState()`.
  // Word must consist of 3+ consecutive letters (or 4 exactly if in the bonus
  // line) and be a valid word in `dict_`. If these conditions are failed,
  // returns an empty string.
  std::string GetWord(const std::vector<Point> &line) const;

  // Solver::IsComplete()
  //
  // Returns `true` if every row of `CurrentState()` has a word on it.
  bool IsComplete() const;

  // Solver::MostRestrictedWordlessRow()
  //
  // Returns the index of the row with the most letters that doesn't have a word
  // in it. Breaks ties in favor of the lowest index.
  int MostRestrictedWordlessRow() const;

  //---------
  // Members

  const Dict dict_;
  const std::vector<std::vector<Point>> lines_;
  const std::vector<Point> bonus_line_;
  const std::vector<Point> multiplier_points_;
  const Gamestate starting_state_;
  std::vector<Gamestate> steps_;
  Gamestate best_state_;
  int best_score_;

  // The number to pass NMostValuableTiles, from which sets of 3 are chosen to
  // make possible bonus words. Note that increasing this n scales by O(n^2).
  int tiles_for_bonus_words_;

  // The number to pass NMostValuableTiles, from which sets are chosen to place
  // on the multiplier tiles. Note that increasing this n scales by O(n^2).
  int tiles_for_multiplier_tiles_;
};

}  // namespace puzzmo::bongo

#endif