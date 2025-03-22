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

  //----------
  // Mutators

  // Solver::reset()
  //
  // Returns the solver to its starting state.
  void reset() { steps_.clear(); }

  // Solver::FillLine()
  //
  // Makes the relevant modification to `CurrentState()`, appending it to
  // `steps_` if successful.
  absl::Status FillLine(const std::vector<Point> &line, absl::string_view word);

  // Solver::Solve()
  //
  // Tries permutations of the highest-scoring tiles on multiplier spaces or
  // bonus path spaces. If no solutions are found, increments the parameter
  // values and tries again.
  absl::StatusOr<Gamestate> Solve();

 private:
  // Solver::FindWordsRecursively()
  //
  // A recursive helper method used by `Solve()`.
  absl::Status FindWordsRecursively();

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
  const Gamestate starting_state_;
  std::vector<Gamestate> steps_;
  Gamestate best_state_;
  int best_score_;

  const std::vector<Point> bonus_line_;
  const std::vector<Point> double_points_;
  const Point triple_point_;

  // The number to pass NMostValuableTiles, from which sets of 3 are chosen to
  // make possible bonus words. Note that increasing this n scales by O(n^2).
  int tiles_for_bonus_words_;

  // The number to pass NMostValuableTiles, from which sets are chosen to place
  // on the multiplier tiles. Note that increasing this n scales by O(n^2).
  int tiles_for_multiplier_tiles_;
};

}  // namespace puzzmo::bongo

#endif