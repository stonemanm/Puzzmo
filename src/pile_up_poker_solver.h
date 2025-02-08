#ifndef pile_up_poker_solver_h
#define pile_up_poker_solver_h

#include <vector>

#include "card.h"

namespace puzzmo {

class PileupPokerSolver {
public:
  explicit PileupPokerSolver(std::vector<Card> cards) : cards_(cards) {};
  std::vector<Card> Solve() const;

private:
  const std::vector<Card> cards_;
  int Score(std::vector<Card> cards) const;
};

} // namespace puzzmo

#endif