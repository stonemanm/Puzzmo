#include "pile_up_poker_solver.h"

#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"

namespace puzzmo {
namespace {

int ScoreHand(const Card &c1, const Card &c2, const Card &c3, const Card &c4,
              bool &count_discard) {
  std::vector<Card> hand = {c1, c2, c3, c4};
  std::sort(hand.begin(), hand.end());

  bool flush = (c1.suit == c2.suit && c2.suit == c3.suit && c3.suit == c4.suit);
  bool straight =
      (hand[1].rank - hand[0].rank == 1 && hand[2].rank - hand[1].rank == 1 &&
       hand[3].rank - hand[2].rank == 1);
  if (straight && flush)
    return 450; // Straight Flush
  if (straight)
    return 180; // Straight
  if (flush)
    return 80; // Flush
  if (hand[0].rank == hand[3].rank)
    return 325; // 4 of a Kind
  if (hand[0].rank == hand[2].rank || hand[1].rank == hand[3].rank)
    return 125; // 3 of a Kind
  if (hand[0].rank == hand[1].rank && hand[2].rank == hand[3].rank)
    return 60; // 2 Pair
  if (hand[0].rank == hand[1].rank || hand[1].rank == hand[2].rank ||
      hand[2].rank == hand[3].rank)
    return 5; // Pair

  count_discard = false;
  return 0; // Non-scoring
}

} // namespace

//

std::vector<Card> PileupPokerSolver::Solve() const {
  int score = 0;
  int best_score = -1;
  std::vector<Card> best_layout;
  std::vector<Card> cards = cards_;

  std::sort(cards.begin(), cards.end());
  do {
    LOG(ERROR) << score
               << absl::StrJoin(cards, ", ",
                                [](std::string *out, const Card &card) {
                                  absl::StrAppend(out, card.toString());
                                }); // temp
    score = Score(cards_);
    if (score > best_score) {
      best_layout = cards;
      best_score = score;

      LOG(ERROR) << " ";
      LOG(ERROR) << "New best score: " << score;
      for (int i = 0; i < 4; ++i) {
        LOG(ERROR) << absl::StrCat(
            "[", cards[4 * i].toString(), " ", cards[4 * i + 1].toString(), " ",
            cards[4 * i + 2].toString(), " ", cards[4 * i + 3].toString(), "]");
      }
    }
  } while (std::next_permutation(cards.begin(), cards.end()));

  return best_layout;
}

int PileupPokerSolver::Score(std::vector<Card> cards) const {
  // Indices in the vector correspond to board spots as follows:
  //
  // 16      0  1  2  3
  // 17      4  5  6  7
  // 18      8  9 10 11
  // 19     12 13 14 15
  //
  // Therefore, these are the hands to be scored, and multipliers
  // - 0, 1, 2, 3
  // - 4, 5, 6, 7
  // - 8, 9, 10, 11
  // - 12, 13, 14, 15
  // - 0, 4, 8, 12
  // - 1, 5, 9, 13
  // - 2, 6, 10, 14
  // - 3, 7, 11, 15
  // - 0, 3, 12, 15     x2
  // - 16, 17, 18, 19   x3 (only if all others score!)
  int score = 0;
  bool count_discard = true;
  score += ScoreHand(cards[0], cards[1], cards[2], cards[3], count_discard);
  score += ScoreHand(cards[4], cards[5], cards[6], cards[7], count_discard);
  score += ScoreHand(cards[8], cards[9], cards[10], cards[11], count_discard);
  score += ScoreHand(cards[12], cards[13], cards[14], cards[15], count_discard);
  score += ScoreHand(cards[0], cards[4], cards[8], cards[12], count_discard);
  score += ScoreHand(cards[1], cards[5], cards[9], cards[13], count_discard);
  score += ScoreHand(cards[2], cards[6], cards[10], cards[14], count_discard);
  score += ScoreHand(cards[3], cards[7], cards[11], cards[15], count_discard);
  score +=
      2 * ScoreHand(cards[0], cards[3], cards[12], cards[15], count_discard);

  if (count_discard) {
    score += 3 * ScoreHand(cards[16], cards[17], cards[18], cards[19],
                           count_discard);
  }

  return score;
}

} // namespace puzzmo