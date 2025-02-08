#include "card.h"

#include <vector>

namespace puzzmo {
namespace {

const std::vector<std::string> kRankStrs = {"A", "2", "3", "4", "5", "6", "7",
                                            "8", "9", "T", "J", "Q", "K"};

const std::vector<std::string> kSuitStrs = {"S", "H", "C", "D"};

} // namespace

std::string Card::toString() const {
  return kRankStrs[rank % 13] + kSuitStrs[suit];
}

bool Card::operator==(const Card &other) const {
  return rank == other.rank && suit == other.suit;
}

bool Card::operator<(const Card &other) const {
  if (rank != other.rank) {
    return rank < other.rank;
  }
  return suit < other.suit;
}

} // namespace puzzmo