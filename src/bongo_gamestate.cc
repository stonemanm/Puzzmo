#include "bongo_gamestate.h"

#include <cctype>

#include "absl/strings/str_cat.h"

namespace puzzmo {

constexpr char kBonusSpace = '*';
constexpr char kDoubleMultiplier = '2';
constexpr char kTripleMultiplier = '3';

BongoGameState::BongoGameState(
    const std::vector<std::string> board_strings,
    const absl::flat_hash_map<char, int> letter_values,
    LetterCount remaining_tiles, std::vector<std::vector<char>> placed_tiles)
    : letter_values_(letter_values), placed_tiles_(placed_tiles),
      remaining_tiles_(remaining_tiles), multipliers_(5, std::vector<int>(5)) {

  // Parse the board_strings to populate multipliers_ and bonus_word_path_
  for (int row = 0; row < 5; ++row) {
    for (int col = 0; col < 5; ++col) {
      const char c = board_strings[row][col];
      switch (c) {
      case kBonusSpace:
        bonus_word_path_.push_back({row, col});
        multipliers_[row][col] = 1;
        break;
      case kDoubleMultiplier:
        multipliers_[row][col] = 2;
        break;
      case kTripleMultiplier:
        multipliers_[row][col] = 3;
        break;
      default:
        multipliers_[row][col] = 1;
        break;
      }
    }
  }
}

bool BongoGameState::PlaceTile(const Point &p, char c) {
  if (!HasPoint(p) || std::isalpha(placed_tiles_[p.row][p.col]) ||
      remaining_tiles_.NumLetters(c) <= 0)
    return false;

  placed_tiles_[p.row][p.col] = c;
  remaining_tiles_.RemoveLetter(c);
  return true;
}

bool BongoGameState::RemoveTile(const Point &p) {
  if (!HasPoint(p))
    return false;

  char c = placed_tiles_[p.row][p.col];
  if (!std::isalpha(placed_tiles_[p.row][p.col]))
    return false;

  remaining_tiles_.AddLetter(c);
  placed_tiles_[p.row][p.col] = '\0';
  return true;
}

std::string BongoGameState::RowWord(int row) const {
  std::string row_word;
  for (int col = 0; col < 5; ++col) {
    char c = placed_tiles_[row][col];
    if (!std::isalpha(c))
      continue;
    absl::StrAppend(&row_word, std::string(1, c));
  }
  return row_word;
}

std::string BongoGameState::BonusWord() const {
  std::string bonus_word;
  for (const Point &p : bonus_word_path_) {
    char c = placed_tiles_[p.row][p.col];
    if (!std::isalpha(c))
      continue;
    absl::StrAppend(&bonus_word, std::string(1, c));
  }
  return bonus_word;
}

int BongoGameState::RowWordScore(int row) const {
  int row_word_score = 0;
  for (int col = 0; col < 5; ++col) {
    char c = placed_tiles_[row][col];
    if (!std::isalpha(c))
      continue;
    row_word_score += letter_values_.at(c) * multipliers_[row][col];
  }
  return row_word_score;
}

int BongoGameState::BonusWordScore() const {
  int bonus_word_score = 0;
  for (const Point &p : bonus_word_path_) {
    char c = placed_tiles_[p.row][p.col];
    if (!std::isalpha(c))
      continue;
    bonus_word_score += letter_values_.at(c) * multipliers_[p.row][p.col];
  }
  return bonus_word_score;
}

bool BongoGameState::HasPoint(const Point &p) const {
  return (0 <= p.row && p.row < 5 && 0 <= p.col && p.col < 5);
}

} // namespace puzzmo
