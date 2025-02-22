#ifndef card_h
#define card_h

#include <string>

namespace puzzmo {

enum Rank {
  kTwo = 1,
  kThree = 2,
  kFour = 3,
  kFive = 4,
  kSix = 5,
  kSeven = 6,
  kEight = 7,
  kNine = 8,
  kTen = 9,
  kJack = 10,
  kQueen = 11,
  kKing = 12,
  kAce = 13
};

enum Suit { kSpades = 0, kHearts = 1, kClubs = 2, kDiamonds = 3 };

struct Card {
  Rank rank;
  Suit suit;

  std::string toString() const;
  bool operator==(const Card &other) const;
  bool operator<(const Card &other) const;
};

}  // namespace puzzmo

#endif