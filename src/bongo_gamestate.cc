#include "bongo_gamestate.h"

#include <cctype>
#include <cmath>
#include <string>

#include "absl/algorithm/container.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"

namespace puzzmo {

constexpr char kBonusSpace = '*';
constexpr char kDoubleMultiplier = '2';
constexpr char kTripleMultiplier = '3';

BongoGameState::BongoGameState(const std::vector<std::string> board_strings,
                               absl::flat_hash_map<char, int> letter_values,
                               LetterCount remaining_tiles,
                               std::vector<std::string> placed_tiles)
    : letter_values_(letter_values),
      multipliers_(5, std::vector<int>(5)),
      placed_tiles_(placed_tiles),
      remaining_tiles_(remaining_tiles) {
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

std::string BongoGameState::NMostValuableTiles(int n) const {
  if (n <= 0) return "";
  std::string letters = remaining_tiles_.CharsInOrder();
  if (letters.size() < n) {
    n = letters.size();
  }
  std::sort(letters.begin(), letters.end(), [this](char l, char r) {
    return letter_values_.at(l) > letter_values_.at(r);
  });
  return letters.substr(0, n);
}

bool BongoGameState::Complete() const {
  for (int row = 0; row < 5; ++row) {
    for (int col = 0; col < 5; ++col) {
      if (!std::isalpha(placed_tiles_[row][col])) {
        return false;
      }
    }
  }
  return true;
}

std::vector<std::string> BongoGameState::TilePlacements() const {
  return placed_tiles_;
}

bool BongoGameState::PlaceTile(const Point &p, char c) {
  if (!HasPoint(p) || std::isalpha(placed_tiles_[p.row][p.col]) ||
      !remaining_tiles_.RemoveLetter(c).ok())
    return false;
  placed_tiles_[p.row][p.col] = c;
  return true;
}

bool BongoGameState::FillRestOfWord(int row, absl::string_view word) {
  if (row < 0 || row >= 5 || word.size() != 5) return false;
  LetterCount needs = LetterCount(word) - LetterCount(RowWord(row));
  if (!needs.Valid() || !remaining_tiles_.Contains(needs)) return false;
  for (int col = 0; col < 5; ++col) {
    if (std::isalpha(placed_tiles_[row][col])) continue;
    placed_tiles_[row][col] = word[col];
  }
  remaining_tiles_ -= needs;
  return true;
}

bool BongoGameState::RemoveTile(const Point &p) {
  if (!HasPoint(p)) return false;

  char c = placed_tiles_[p.row][p.col];
  if (!std::isalpha(c) || !remaining_tiles_.AddLetter(c).ok()) return false;
  placed_tiles_[p.row][p.col] = '_';
  return true;
}

bool BongoGameState::ClearRowExceptBonusTiles(int row) {
  if (row < 0 || row >= 5) return false;
  for (int col = 0; col < 5; ++col) {
    Point p = {row, col};
    char c = placed_tiles_[row][col];
    if (absl::c_linear_search(bonus_word_path_, p) ||
        multipliers_[row][col] > 1 || !std::isalpha(c) ||
        !remaining_tiles_.AddLetter(c).ok())
      continue;
    placed_tiles_[row][col] = '_';
  }
  return true;
}

bool BongoGameState::ClearBoardExceptBonusTiles() {
  for (int row = 0; row < 5; ++row) {
    for (int col = 0; col < 5; ++col) {
      Point p = {row, col};
      char c = placed_tiles_[row][col];
      if (absl::c_linear_search(bonus_word_path_, p) ||
          multipliers_[row][col] > 1 || !std::isalpha(c) ||
          !remaining_tiles_.AddLetter(c).ok())
        continue;
      placed_tiles_[row][col] = '_';
    }
  }
  return true;
}

std::vector<Point> BongoGameState::MultiplierSquares() const {
  std::vector<Point> mult_squares;
  for (int row = 0; row < 5; ++row) {
    for (int col = 0; col < 5; ++col) {
      if (multipliers_[row][col] >= 2) {
        mult_squares.push_back({row, col});
      }
    }
  }
  return mult_squares;
}

std::vector<Point> BongoGameState::BonusWordPath() const {
  return bonus_word_path_;
}

std::string BongoGameState::RowWord(int row) const {
  if (row < 0 || row >= 5) return "";
  std::string row_word;
  for (int col = 0; col < 5; ++col) {
    char c = placed_tiles_[row][col];
    if (!std::isalpha(c)) continue;
    absl::StrAppend(&row_word, std::string(1, c));
  }
  return row_word;
}

std::string BongoGameState::BonusWord() const {
  std::string bonus_word;
  for (const Point &p : bonus_word_path_) {
    char c = placed_tiles_[p.row][p.col];
    if (!std::isalpha(c)) continue;
    absl::StrAppend(&bonus_word, std::string(1, c));
  }
  return bonus_word;
}

int BongoGameState::BonusWordScore(const BongoDictionary &dict) const {
  if (!dict.IsValidWord(BonusWord())) return 0;
  int score = 0;
  for (const Point &p : bonus_word_path_) {
    char c = placed_tiles_[p.row][p.col];
    if (!std::isalpha(c)) continue;
    score += letter_values_.at(c) * multipliers_[p.row][p.col];
  }
  return std::ceil(score * (dict.IsCommonWord(BonusWord()) ? 1.3 : 1));
}

int BongoGameState::RowWordScore(int row, const BongoDictionary &dict) const {
  if (row < 0 || row >= 5 || !dict.IsValidWord(RowWord(row))) return 0;
  int score = 0;
  for (int col = 0; col < 5; ++col) {
    if (char c = placed_tiles_[row][col]; std::isalpha(c)) {
      score += (letter_values_.at(c) * multipliers_[row][col]);
    }
  }
  return std::ceil(score * (dict.IsCommonWord(RowWord(row)) ? 1.3 : 1));
}

int BongoGameState::Score(const BongoDictionary &dict) const {
  return RowWordScore(0, dict) + RowWordScore(1, dict) + RowWordScore(2, dict) +
         RowWordScore(3, dict) + RowWordScore(4, dict) + BonusWordScore(dict);
}

std::string BongoGameState::RowRegex(int row) const {
  if (row < 0 || row >= 5) return "";
  std::string rgx = "";
  for (int col = 0; col < 5; ++col) {
    char c = placed_tiles_[row][col];
    absl::StrAppend(&rgx, std::isalpha(c) ? std::string(1, c)
                                          : remaining_tiles_.AnyCharRegex());
  }
  return rgx;
}

int BongoGameState::IndexOfFullestIncompleteRow() const {
  int i = 0;
  int most_letters = -1;
  for (int row = 0; row < 5; ++row) {
    int letters = absl::c_count_if(placed_tiles_[row],
                                   [](char c) { return std::isalpha(c); });
    if (letters > most_letters && letters < 5) {
      most_letters = letters;
      i = row;
    }
  }
  return i;
}

bool BongoGameState::HasPoint(const Point &p) const {
  return (0 <= p.row && p.row < 5 && 0 <= p.col && p.col < 5);
}

bool operator==(const BongoGameState &lhs, const BongoGameState &rhs) {
  return lhs.bonus_word_path_ == rhs.bonus_word_path_ &&
         lhs.letter_values_ == rhs.letter_values_ &&
         lhs.multipliers_ == rhs.multipliers_ &&
         lhs.placed_tiles_ == rhs.placed_tiles_ &&
         lhs.remaining_tiles_ == rhs.remaining_tiles_;
}

bool operator<(const BongoGameState &lhs, const BongoGameState &rhs) {
  return !(lhs.bonus_word_path_ == rhs.bonus_word_path_)
             ? (lhs.bonus_word_path_ < rhs.bonus_word_path_)
         : !(lhs.multipliers_ == rhs.multipliers_)
             ? (lhs.multipliers_ < rhs.multipliers_)
         : !(lhs.placed_tiles_ == rhs.placed_tiles_)
             ? (lhs.placed_tiles_ < rhs.placed_tiles_)
         : !(lhs.remaining_tiles_ == rhs.remaining_tiles_)
             ? (lhs.remaining_tiles_ < rhs.remaining_tiles_)
             : false;
}

}  // namespace puzzmo
