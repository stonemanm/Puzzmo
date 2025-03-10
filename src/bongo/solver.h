// -----------------------------------------------------------------------------
// File: solver.h
// -----------------------------------------------------------------------------
//
// This header file defines the solver class for Bongo. It contains a
// dictionary, helper methods to interact with the board using the dictionary,
// and various solution methods.

#ifndef solver_h
#define solver_h

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

  // Solver::CeilingForScore()
  //
  // Returns the highest possible score for the gamestate with the assumption
  // that every string is a common word.
  int CeilingForScore() const;

  //----------
  // Mutators

  // Solver::reset()
  //
  // Returns the solver to its starting state.
  void reset();

  absl::StatusOr<Gamestate> FindSolutionWithScore(int score) const;

  absl::StatusOr<Gamestate> Solve();

 private:
  absl::Status FindWordsRecursively(Gamestate &current_board);
  int Score(const Gamestate &bgs) const;
  int PathScore(const Gamestate &bgs, const std::vector<Point> &path) const;

  const Dict dict_;
  const Gamestate starting_state_;
  Gamestate state_;
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