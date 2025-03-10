#ifndef PUZZMO_PILEUPPOKER_PILEUPPOKERSOLVER_H_
#define PUZZMO_PILEUPPOKER_PILEUPPOKERSOLVER_H_

#include <vector>

#include "src/pileuppoker/card.h"

namespace puzzmo {

class PileupPokerSolver {
 public:
  explicit PileupPokerSolver(std::vector<Card> cards) : cards_(cards) {};
  std::vector<Card> Solve() const;

 private:
  const std::vector<Card> cards_;
  int Score(std::vector<Card> cards) const;
};

}  // namespace puzzmo

#endif