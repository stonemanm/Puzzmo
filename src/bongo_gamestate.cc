#include "bongo_gamestate.h"

#include <cctype>
#include <climits>
#include <cmath>
#include <string>

#include "absl/algorithm/container.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"

namespace puzzmo {

constexpr char kBonusSpace = '*';
constexpr char kDoubleMultiplier = '2';
constexpr char kTripleMultiplier = '3';

namespace {

bool HasPoint(const Point &p) {
  return (0 <= p.row && p.row < 5 && 0 <= p.col && p.col < 5);
}

bool HasRow(int row) { return (0 <= row && row < 5); }

std::string LongestAlphaSubstring(absl::string_view s) {
  int best_start = 0;
  int best_len = 0;
  int start = 0;
  int len = 0;
  for (int i = 0; i < s.size(); ++i) {
    if (std::isalpha(s[i])) {
      ++len;
      if (len > best_len) {
        best_start = start;
        best_len = len;
      }
    }
    if (!std::isalpha(s[i])) {
      start = i + 1;
      len = 0;
    }
  }
  return std::string(s.substr(best_start, best_len));
}

}  // namespace

BongoGameState::BongoGameState(const std::vector<std::string> board,
                               absl::flat_hash_map<char, int> letter_values,
                               LetterCount letters_remaining,
                               std::vector<std::string> letter_grid)
    : letters_remaining_(letters_remaining),
      letter_grid_(std::vector<std::string>(5, std::string(5, '_'))),
      multiplier_grid_(5, std::vector<int>(5)),
      letter_values_(letter_values),
      is_locked_(5, std::vector<bool>(5)) {
  // Parse the inputs one space at a time.
  for (int row = 0; row < 5; ++row) {
    for (int col = 0; col < 5; ++col) {
      if (row < letter_grid.size() && col < letter_grid[row].size())
        set_char_at(row, col, letter_grid[row][col]);
      switch (board[row][col]) {
        case kBonusSpace:
          bonus_word_path_.push_back({row, col});
          multiplier_grid_[row][col] = 1;
          break;
        case kDoubleMultiplier:
          multiplier_grid_[row][col] = 2;
          break;
        case kTripleMultiplier:
          multiplier_grid_[row][col] = 3;
          break;
        default:
          multiplier_grid_[row][col] = 1;
          break;
      }
    }
  }
}

absl::Status BongoGameState::PlaceLetter(const Point &p, char c) {
  if (!HasPoint(p))
    return absl::OutOfRangeError(
        absl::StrCat("Point ", p, " is outside of the gamestate.\n", *this));
  if (auto s = letters_remaining_.RemoveLetter(c); !s.ok()) return s.status();
  if (char x = char_at(p); std::isalpha(x)) {
    auto s = letters_remaining_.AddLetter(x);
    if (!s.ok()) return s.status();
  }
  if (is_locked_at(p))
    return absl::InvalidArgumentError(absl::StrCat(
        "Point ", p, " is locked and cannot be overwritten. Unlock it first.\n",
        *this));

  set_char_at(p, c);
  return absl::OkStatus();
}

absl::Status BongoGameState::PlaceWord(absl::string_view word, int row,
                                       int starting_col) {
  if (!HasRow(row))
    return absl::OutOfRangeError(absl::StrCat(
        "Row ", row, " falls outside the bounds of the Bongo board.\n", *this));
  if (!HasRow(starting_col))
    return absl::OutOfRangeError(
        absl::StrCat("Column ", starting_col,
                     " falls outside the bounds of the Bongo board.\n", *this));
  if (word.length() > 5 - starting_col)
    return absl::InvalidArgumentError(
        absl::StrCat("Word ", word, " has length ", word.length(),
                     ", which cannot fit in the ", 5 - starting_col,
                     " remaining spaces.\n", *this));

  LetterCount needs = LetterCount(word) - LetterCount(row_string(row));
  if (!letters_remaining().contains(needs))
    return absl::InvalidArgumentError(
        absl::StrCat("Insufficient tiles remain to place word ", word,
                     " in row ", row_string(row), "\n", *this));

  // Check if any of the important slots are locked, and if so, if they need to
  // be overwritten.
  for (int col = starting_col; col < 5; ++col) {
    if (is_locked_at(row, col) &&
        char_at(row, col) != word[col - starting_col]) {
      Point p = {row, col};
      return absl::InvalidArgumentError(absl::StrCat(
          "Point ", p,
          " is locked and cannot be overwritten. Unlock it first.\n", *this));
    }
  }

  for (int col = starting_col; col < 5; ++col) {
    char c = word[col - starting_col];
    if (is_locked_at(row, col)) continue;
    auto s = PlaceLetter({row, col}, c);
    if (!s.ok()) return s;
  }
  return absl::OkStatus();
}

absl::Status BongoGameState::PlaceBonusWord(absl::string_view word) {
  LetterCount needs = LetterCount(word) - LetterCount(bonus_string());
  if (!letters_remaining_.contains(needs))
    return absl::InvalidArgumentError(
        absl::StrCat("Insufficient tiles remain to place word ", word,
                     " in bonus ", bonus_string(), "\n", *this));

  // Check if any of the important slots are locked, and if so, if they need
  // to be overwritten.
  for (int i = 0; i < bonus_word_path_.size(); ++i) {
    const Point p = bonus_word_path_[i];
    if (is_locked_at(p) && char_at(p) != word[i]) {
      return absl::InvalidArgumentError(absl::StrCat(
          "Point ", p,
          " is locked and cannot be overwritten. Unlock it first.\n", *this));
    }
  }

  for (int i = 0; i < word.size(); ++i) {
    const Point p = bonus_word_path_[i];
    if (is_locked_at(p)) continue;
    if (auto s = PlaceLetter(p, word[i]); !s.ok()) return s;
    set_is_locked_at(p, true);
  }
  return absl::OkStatus();
}

absl::Status BongoGameState::RemoveLetter(const Point &p) {
  if (!HasPoint(p))
    return absl::OutOfRangeError(
        absl::StrCat("Point ", p, " is outside of the gamestate.\n", *this));
  if (is_locked_at(p))
    return absl::InvalidArgumentError(absl::StrCat(
        "Point ", p, " is locked and cannot be removed. Unlock it first.\n",
        *this));
  if (char x = char_at(p); std::isalpha(x)) {
    auto s = letters_remaining_.AddLetter(x);
    if (!s.ok()) return s.status();
  }

  set_char_at(p, '_');
  return absl::OkStatus();
}

absl::Status BongoGameState::RemoveWord(int row) {
  if (!HasRow(row))
    return absl::OutOfRangeError(absl::StrCat(
        "Row ", row, " falls outside the bounds of the Bongo board.\n", *this));
  for (int col = 0; col < 5; ++col) {
    if (is_locked_at(row, col)) continue;
    auto s = RemoveLetter({row, col});
    if (!s.ok()) return s;
  }
  return absl::OkStatus();
}

void BongoGameState::ClearBoard() {
  for (int row = 0; row < 5; ++row) {
    for (int col = 0; col < 5; ++col) {
      if (is_locked_at(row, col)) continue;
      (void)RemoveLetter({row, col});
    }
  }
}

std::string BongoGameState::RowWord(int row) const {
  if (row < 0 || row >= 5) return "";
  std::string s = LongestAlphaSubstring(row_string(row));
  return s.length() >= 3 ? s : "";
}

std::string BongoGameState::BonusWord() const {
  std::string s = bonus_string();
  return LetterCount(s).size() == 4 ? s : "";
}

int BongoGameState::MostRestrictedWordlessRow() const {
  int row = 0;
  int most_letters = INT_MIN;
  for (int i = 0; i < 5; ++i) {
    if (!RowWord(i).empty()) continue;
    int letters = LetterCount(row_string(i)).size();
    if (letters > most_letters && letters < 5) {
      most_letters = letters;
      row = i;
    }
  }
  return row;
}

bool BongoGameState::Complete() const {
  for (int row = 0; row < 5; ++row) {
    if (RowWord(row).empty()) return false;
  }
  return true;
}

std::vector<Point> BongoGameState::MultiplierSquares() const {
  std::vector<Point> mult_squares;
  for (int row = 0; row < 5; ++row) {
    for (int col = 0; col < 5; ++col) {
      if (multiplier_grid_[row][col] >= 2) {
        mult_squares.push_back({row, col});
      }
    }
  }
  return mult_squares;
}

std::string BongoGameState::NMostValuableTiles(int n) const {
  if (n <= 0) return "";

  std::string letters = letters_remaining_.CharsInOrder();
  if (letters.size() < n) return letters;

  std::sort(letters.begin(), letters.end(), [this](char l, char r) {
    return letter_values_.at(l) > letter_values_.at(r);
  });
  return letters.substr(0, n);
}

// TODO: make things callable on rows or bonus alike by passing in a vector of
// points.
std::string BongoGameState::RegexForRow(int row) const {
  // TODO: Add the ability to search for 4 letter words.
  std::string rgx = "";
  std::string s = row_string(row);
  if (row < 0 || row >= 5 || LongestAlphaSubstring(s).empty()) return rgx;
  for (char c : s) {
    absl::StrAppend(&rgx, std::isalpha(c)
                              ? std::string(1, c)
                              : letters_remaining_.RegexMatchingContents());
  }
  return rgx;
}

std::string BongoGameState::RegexForBonus() const {
  std::string rgx = "";
  std::string s = bonus_string();
  if (LongestAlphaSubstring(s).empty()) return rgx;
  for (char c : s) {
    absl::StrAppend(&rgx, std::isalpha(c)
                              ? std::string(1, c)
                              : letters_remaining_.RegexMatchingContents());
  }
  return rgx;
}

int BongoGameState::CalculateScore(const BongoDictionary &dict) const {
  return CalculateRowScore(0, dict) + CalculateRowScore(1, dict) +
         CalculateRowScore(2, dict) + CalculateRowScore(3, dict) +
         CalculateRowScore(4, dict) + CalculateBonusScore(dict);
}

int BongoGameState::CalculateRowScore(int row,
                                      const BongoDictionary &dict) const {
  if (row < 0 || row >= 5 || !dict.IsValidWord(RowWord(row))) return 0;
  int score = 0;
  for (int col = 0; col < 5; ++col) {
    if (char c = letter_grid_[row][col]; std::isalpha(c)) {
      score += (letter_values_.at(c) * multiplier_grid_[row][col]);
    }
  }
  return std::ceil(score * (dict.IsCommonWord(RowWord(row)) ? 1.3 : 1));
}

int BongoGameState::CalculateBonusScore(const BongoDictionary &dict) const {
  if (!dict.IsValidWord(BonusWord())) return 0;
  int score = 0;
  for (const Point &p : bonus_word_path_) {
    char c = letter_grid_[p.row][p.col];
    if (!std::isalpha(c)) continue;
    score += letter_values_.at(c) * multiplier_grid_[p.row][p.col];
  }
  return std::ceil(score * (dict.IsCommonWord(BonusWord()) ? 1.3 : 1));
}

/** * * * * * * * * * * *
 * Accessors & mutators *
 * * * * * * * * * * * **/

void BongoGameState::set_letters_remaining(LetterCount lc) {
  letters_remaining_ = lc;
}

LetterCount BongoGameState::letters_remaining() const {
  return letters_remaining_;
}

void BongoGameState::set_letter_grid(std::vector<std::string> grid) {
  letter_grid_ = grid;
}

std::vector<std::string> BongoGameState::letter_grid() const {
  return letter_grid_;
}

void BongoGameState::set_row_string(int row, absl::string_view sv) {
  if (!HasRow(row)) return;
  letter_grid_[row] = sv.substr(0, 5);
}

std::string BongoGameState::row_string(int row) const {
  return HasRow(row) ? letter_grid_[row] : "";
}

void BongoGameState::set_char_at(const Point &p, char c) {
  if (!HasPoint(p)) return;
  letter_grid_[p.row][p.col] = c;
}

char BongoGameState::char_at(const Point &p) const {
  return HasPoint(p) ? letter_grid_[p.row][p.col] : '\0';
}

void BongoGameState::set_multiplier_grid(std::vector<std::vector<int>> grid) {
  multiplier_grid_ = grid;
}

std::vector<std::vector<int>> BongoGameState::multiplier_grid() const {
  return multiplier_grid_;
}

void BongoGameState::set_multiplier_at(const Point &p, int i) {
  if (!HasPoint(p)) return;
  multiplier_grid_[p.row][p.col] = i;
}

int BongoGameState::multiplier_at(const Point &p) const {
  if (!HasPoint(p)) return 0;
  return multiplier_grid_[p.row][p.col];
}

void BongoGameState::set_bonus_word_path(std::vector<Point> path) {
  bonus_word_path_ = path;
}

std::vector<Point> BongoGameState::bonus_word_path() const {
  return bonus_word_path_;
}

void BongoGameState::set_bonus_string(absl::string_view sv) {
  for (int i = 0; i < bonus_word_path_.size(); ++i) {
    if (sv.size() <= i) break;
    set_char_at(bonus_word_path_[i], sv[i]);
  }
}

std::string BongoGameState::bonus_string() const {
  std::string s = "";
  for (int i = 0; i < bonus_word_path_.size(); ++i) {
    s += char_at(bonus_word_path_[i]);
  }
  return s;
}

void BongoGameState::set_letter_values(absl::flat_hash_map<char, int> lvmap) {
  letter_values_ = lvmap;
}

absl::flat_hash_map<char, int> BongoGameState::letter_values() const {
  return letter_values_;
}

void BongoGameState::set_is_locked(std::vector<std::vector<bool>> is_locked) {
  is_locked_ = is_locked;
}
std::vector<std::vector<bool>> BongoGameState::is_locked() const {
  return is_locked_;
}

void BongoGameState::set_is_locked_at(const Point &p, bool is_locked) {
  if (!HasPoint(p)) return;
  is_locked_[p.row][p.col] = is_locked;
}

bool BongoGameState::is_locked_at(const Point &p) const {
  if (!HasPoint(p)) return false;
  return is_locked_[p.row][p.col];
}

/** * * * * * * * * * * *
 * Overloaded operators *
 * * * * * * * * * * * **/

bool operator==(const BongoGameState &lhs, const BongoGameState &rhs) {
  return lhs.letters_remaining() == rhs.letters_remaining() &&
         lhs.letter_grid() == rhs.letter_grid() &&
         lhs.multiplier_grid() == rhs.multiplier_grid() &&
         lhs.bonus_word_path() == rhs.bonus_word_path() &&
         lhs.letter_values() == rhs.letter_values() &&
         lhs.is_locked() == rhs.is_locked();
}

bool operator!=(const BongoGameState &lhs, const BongoGameState &rhs) {
  return !(lhs == rhs);
}

}  // namespace puzzmo
