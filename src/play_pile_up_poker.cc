#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "src/pileuppoker/pile_up_poker_solver.h"

using namespace puzzmo;

int main(int argc, const char *argv[]) {
  // Read in the board
  std::vector<Card> cards;
  std::ifstream cardfile("inputs/pile_up_poker_cards.txt");
  if (!cardfile.is_open()) {
    LOG(ERROR) << "Error: Could not open pile_up_poker_cards.txt";
    return 1;
  }
  std::string line;
  while (std::getline(cardfile, line)) {
    Card card;
    std::istringstream iss(line);
    char c;
    while (iss >> c) {
      switch (c) {
        case 'A':
          card.rank = kAce;
          break;
        case 'K':
          card.rank = kKing;
          break;
        case 'Q':
          card.rank = kQueen;
          break;
        case 'J':
          card.rank = kJack;
          break;
        case 'T':
          card.rank = kTen;
          break;
        case '9':
          card.rank = kNine;
          break;
        case '8':
          card.rank = kEight;
          break;
        case '7':
          card.rank = kSeven;
          break;
        case '6':
          card.rank = kSix;
          break;
        case '5':
          card.rank = kFive;
          break;
        case '4':
          card.rank = kFour;
          break;
        case '3':
          card.rank = kThree;
          break;
        case '2':
          card.rank = kTwo;
          break;
        case 'S':
          card.suit = kSpades;
          break;
        case 'H':
          card.suit = kHearts;
          break;
        case 'C':
          card.suit = kClubs;
          break;
        case 'D':
          card.suit = kDiamonds;
          break;
      }
    }
    cards.push_back(card);
  }
  cardfile.close();

  PileupPokerSolver solver(cards);
  std::vector<Card> solution = solver.Solve();

  for (int i = 0; i < 4; ++i) {
    LOG(ERROR) << absl::StrCat("[", solution[4 * i].toString(), " ",
                               solution[4 * i + 1].toString(), " ",
                               solution[4 * i + 2].toString(), " ",
                               solution[4 * i + 3].toString(), "]");
  }
  return 0;
}