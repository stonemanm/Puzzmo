#ifndef solver_h
#define solver_h

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "dictionary.h"
#include "gamestate.h"

namespace puzzmo::bongo {

class Solver {
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

  Solver(const Dictionary &dict, const Gamestate &bgs,
         BongoSolverOptions options)
      : dict_(dict),
        starting_state_(bgs),
        highest_scoring_board_(bgs),
        tiles_for_bonus_words_(options.tiles_for_bonus_words),
        tiles_for_multiplier_tiles_(options.tiles_for_multiplier_tiles) {}

  int CeilingForScore() const;
  absl::StatusOr<Gamestate> FindSolutionWithScore(int score) const;

  absl::StatusOr<Gamestate> Solve();

 private:
  absl::Status FindWordsRecursively(Gamestate &current_board);
  int Score(const Gamestate &bgs) const;
  int PathScore(const Gamestate &bgs, const std::vector<Point> &path) const;

  const Dictionary dict_;
  const Gamestate starting_state_;
  Gamestate highest_scoring_board_;
  int highest_score_;

  // The number to pass NMostValuableTiles, from which sets of 3 are chosen to
  // make possible bonus words. Note that increasing this n scales by O(n^2).
  int tiles_for_bonus_words_;

  // The number to pass NMostValuableTiles, from which sets are chosen to place
  // on the multiplier tiles. Note that increasing this n scales by O(n^2).
  int tiles_for_multiplier_tiles_;
};

}  // namespace puzzmo::bongo

#endif