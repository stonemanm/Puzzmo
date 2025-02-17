#ifndef bongo_solver_h
#define bongo_solver_h

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "bongo_dictionary.h"
#include "bongo_gamestate.h"

namespace puzzmo {

class BongoSolver {
 public:
  struct BongoSolverOptions {
    // The number to pass NMostValuableTiles, from which sets of 3 are chosen to
    // make possible bonus words. Note that increasing this n scales by O(n^2).
    int tiles_for_bonus_words = 7;

    // The number to pass NMostValuableTiles, from which sets are chosen to
    // place on the multiplier tiles. Note that increasing this n scales by
    // O(n^2).
    int tiles_for_multiplier_tiles = 4;
  };

  BongoSolver(const BongoDictionary &dict, const BongoGameState &bgs,
              BongoSolverOptions options)
      : dict_(dict),
        starting_state_(bgs),
        highest_scoring_board_(bgs),
        tiles_for_bonus_words_(options.tiles_for_bonus_words),
        tiles_for_multiplier_tiles_(options.tiles_for_multiplier_tiles) {}

  absl::StatusOr<BongoGameState> Solve();

 private:
  absl::Status FindWordsRecursively(BongoGameState &current_board);

  const BongoDictionary dict_;
  const BongoGameState starting_state_;
  BongoGameState highest_scoring_board_;

  // The number to pass NMostValuableTiles, from which sets of 3 are chosen to
  // make possible bonus words. Note that increasing this n scales by O(n^2).
  int tiles_for_bonus_words_;

  // The number to pass NMostValuableTiles, from which sets are chosen to place
  // on the multiplier tiles. Note that increasing this n scales by O(n^2).
  int tiles_for_multiplier_tiles_;
};

}  // namespace puzzmo

#endif