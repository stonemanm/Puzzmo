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
#include "dict.h"
#include "gamestate.h"

namespace puzzmo::bongo {

// bongo::Technique
//
// Each enumerated `Technique` corresponds to (a) a set of cells in the grid and
// (b) a `Solver::OptionsFor*()` class method in `bongo::Solver` that returns
// permutations of letters to place on them.
enum class Technique {

  // Technique::kFillMostRestrictedRow
  //
  // The default technique. Tries options for whatever row has the most
  // populated cells without having a word in it. Corresponds to
  // `Solver::OptionsForLine()`.
  kFillMostRestrictedRow = 0,

  // Technique::kFillBonusWordCells
  //
  // Try to fill the bonus line. Possible options are constrained by
  // `Parameters::num_tiles_for_bonus_words`. Corresponds to
  // `Solver::OptionsForBonusWord()`.
  kFillBonusWordCells,

  // Technique::kFillMultiplierCells
  //
  // Try to fill the cells with multipliers. Possible options are constrained by
  // `Parameters::num_tiles_for_mult_cells`. Corresponds to
  // `Solver::OptionsForMultiplierTiles()`.
  kFillMultiplierCells,
};

// bongo::Solver
//
// The `Solver` class is used to output possible solutions to a Bongo puzzle. It
// is the highest-level class in the library.
class Solver {
 public:
  struct Parameters {
    // The techniques to apply, in order.
    std::vector<Technique> techniques = {Technique::kFillBonusWordCells,
                                         Technique::kFillMultiplierCells};

    // The number to pass NMostValuableTiles, from which sets of 3 are chosen to
    // make possible bonus words. Note that increasing this n scales by O(n^2).
    int num_tiles_for_bonus_words = 7;

    // The number to pass NMostValuableTiles, from which sets are chosen to
    // place on the multiplier tiles. Note that increasing this n scales by
    // O(n^2).
    int num_tiles_for_mult_cells = 4;
  };

  /** * * * * * * *
   * Constructors *
   * * * * * * * **/

  // The simplest constructor for a `Solver`, taking a `Dict`, a `Gamestate`,
  // and some `Parameters`. For simplicity, the grid can be provided in the form
  // of `grid_strings`.
  Solver(const Dict &dict, const Gamestate &state, Parameters params);

  /** * * * * **
   * Accessors *
   ** * * * * **/

  // Solver::dict()
  //
  // Provides access to the underlying `Dict`.
  Dict dict() const { return dict_; }

  // Solver::starting_state()
  //
  // Returns the starting gamestate passed to the solver.
  Gamestate starting_state() const { return starting_state_; }

  // Solver::state()
  //
  // Returns the current gamestate managed by the solver.
  Gamestate state() const { return state_; }

  // Solver::best_score()
  //
  // Returns the highest score of any `Gamestate` seen by the solver.
  int best_score() const { return best_score_; }

  // Solver::best_state()
  //
  // Returns the `Gamestate` that scored `best_score`.
  Gamestate best_state() const { return best_state_; }

  /** * * * * *
   * Mutators *
   * * * * * **/

  // Solver::reset()
  //
  // Returns the solver to its starting state.
  void reset();

  // Solver::Solve()
  //
  // Applies `techniques_` in order, depth-first, ending each branch when
  // `IsComplete()` is `true`. If that point has not been reached once all
  // provided `techniques` have been applied, repeatedly applies
  // `Technique::kFillMostRestrictedRow` to finish the search. At the end of
  // every branch of the search, if the current gamestate scores higher than
  // `best_score_`, it is saved as `best_state_` and `best_score_` is updated.
  //
  // If no solutions are found, the process is repeated from the top with
  // `tiles_for_bonus_words_` and `tiles_for_multiplier_tiles_` both
  // incremented.
  absl::StatusOr<Gamestate> Solve();

 private:
  // Solver::RecursiveHelper()
  //
  // If the gamestate is complete, checks to see if we have a new best state.
  // Otherwise, applies the `i`th technique in `techniques_`, gathering the
  // corresponding options and calling the corresponding filler method, and then
  // calls itself with `i` incremented.
  //
  // After all techniques in the vector have been used, the technique
  // `Technique::kFillMostRestrictedRow` will be used to finish the boards.
  absl::Status RecursiveHelper(int i);

  // Solver::RemainingMultiplierCells()
  //
  // A simple helper function that returns `multiplier_cells_` without any that
  // already contain letters.
  std::vector<Point> RemainingMultiplierCells() const;

  // Solver::FillCells()
  //
  // Makes the relevant modification to `state_`.
  absl::Status FillCells(const std::vector<Point> &cells,
                         const absl::string_view letters);

  // Solver::ClearCells()
  //
  // Unlocks the cells most recent modified in `state_` and clears them.
  absl::Status ClearCells();

  /** * * * **
   * Options *
   ** * * * **/

  // Solver::OptionsForBonusWord()
  //
  // Calculates and returns all words to be tried in the bonus word slot. In
  // addition to the available letters, options are limited by the dictionary,
  // the letters already placed in the bonus word, and by
  // `Parameters::tiles_for_bonus_words`.
  absl::flat_hash_set<std::string> OptionsForBonusWord() const;

  // Solver::OptionsForLine()
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

  /** * * * **
   * Scoring *
   ** * * * **/

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
  // Checks `state_` against `best_state_`, updating `best_state_` and
  // `best_score_` if the current state scores higher.
  void UpdateBestState();

  /** * * **
   * Words *
   ** * * **/

  // Solver::GetWord()
  //
  // Returns the word that will be scored from the line in `state_`.
  // Word must consist of 3+ consecutive letters (or 4 exactly if in the bonus
  // line) and be a valid word in `dict_`. If these conditions are failed,
  // returns an empty string.
  std::string GetWord(const std::vector<Point> &line) const;

  // Solver::IsComplete()
  //
  // Returns `true` if every row of `state_` has a word on it.
  bool IsComplete() const;

  // Solver::MostRestrictedWordlessRow()
  //
  // Returns the index of the row with the most letters that doesn't have a word
  // in it. Breaks ties in favor of the lowest index.
  int MostRestrictedWordlessRow() const;

  /** * * * **
   * Members *
   ** * * * **/

  const Dict dict_;
  const std::vector<std::vector<Point>> lines_;
  const std::vector<Point> bonus_line_;
  const std::vector<Point> multiplier_points_;
  const Gamestate starting_state_;
  int best_score_;
  Gamestate best_state_;
  Gamestate state_;
  std::vector<absl::flat_hash_set<Point>> locks_;
  Parameters params_;
};

}  // namespace puzzmo::bongo

#endif