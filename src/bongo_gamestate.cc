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

absl::Status BongoGameState::PlaceString(absl::string_view word,
                                         const std::vector<Point> &path) {
  // Ensure the word and path are of equal size.
  if (word.length() != path.size())
    return absl::InvalidArgumentError(
        absl::StrCat("Word has length ", word.length(), " but path has size ",
                     path.size(), ".\n", *this));

  // Make sure there are enough letters to place the word.
  std::string path_str = path_string(path);
  LetterCount needs = LetterCount(word) - LetterCount(path_str);
  if (!letters_remaining_.contains(needs))
    return absl::InvalidArgumentError(
        absl::StrCat("Insufficient tiles remain to place word ", word, " in ",
                     path_str, "\n", *this));

  // Check if any of the important slots are locked, and if so, if they need
  // to be overwritten.
  for (int i = 0; i < path.size(); ++i) {
    const Point p = path[i];
    if (is_locked_at(p) && path_str[i] != word[i]) {
      return absl::InvalidArgumentError(absl::StrCat(
          "Point ", p,
          " is locked and cannot be overwritten. Unlock it first.\n", *this));
    }
  }

  // Place letters to complete the word.
  for (int i = 0; i < path.size(); ++i) {
    const Point p = path[i];
    if (is_locked_at(p)) continue;
    if (auto s = PlaceLetter(p, word[i]); !s.ok()) return s;
  }
  return absl::OkStatus();
}

absl::Status BongoGameState::ClearLetter(const Point &p) {
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

absl::Status BongoGameState::ClearString(const std::vector<Point> &path) {
  for (const Point &p : path) {
    if (is_locked_at(p)) continue;
    auto s = ClearLetter({p});
    if (!s.ok()) return s;
  }
  return absl::OkStatus();
}

absl::Status BongoGameState::ClearBoard() {
  for (int row = 0; row < 5; ++row) {
    for (int col = 0; col < 5; ++col) {
      if (is_locked_at(row, col)) continue;
      auto s = ClearLetter({row, col});
      if (!s.ok()) return s;
    }
  }
  return absl::OkStatus();
}

std::string BongoGameState::GetWord(const std::vector<Point> &path) const {
  int threshold = (path == bonus_word_path_) ? 4 : 3;
  std::string path_substr = LongestAlphaSubstring(path_string(path));
  return (path_substr.length() >= threshold) ? path_substr : "";
}

int BongoGameState::MostRestrictedWordlessRow() const {
  int row_to_focus = 0;
  int most_letters = INT_MIN;
  for (int row = 0; row < 5; ++row) {
    if (!GetWord(row_path(row)).empty()) continue;
    int letters = LetterCount(row_string(row)).size();
    if (letters > most_letters) {
      most_letters = letters;
      row_to_focus = row;
    }
  }
  return row_to_focus;
}

bool BongoGameState::Complete() const {
  for (int row = 0; row < 5; ++row) {
    if (GetWord(row_path(row)).empty()) return false;
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

std::string BongoGameState::RegexForPath(const std::vector<Point> &path) const {
  // TODO: Add the ability to search for 4 letter words.

  std::string s = path_string(path);
  if (LetterCount(s).empty()) return "";

  std::string rgx = "";
  for (char c : s) {
    absl::StrAppend(&rgx, std::isalpha(c)
                              ? std::string(1, c)
                              : letters_remaining_.RegexMatchingContents());
  }
  return rgx;
}

int BongoGameState::CalculateScore(const BongoDictionary &dict) const {
  return CalculatePathScore(row_path(0), dict) +
         CalculatePathScore(row_path(1), dict) +
         CalculatePathScore(row_path(2), dict) +
         CalculatePathScore(row_path(3), dict) +
         CalculatePathScore(row_path(4), dict) +
         CalculatePathScore(bonus_word_path(), dict);
}

int BongoGameState::CalculatePathScore(const std::vector<Point> &path,
                                       const BongoDictionary &dict) const {
  std::string word = GetWord(path);
  if (!dict.IsValidWord(word)) return 0;

  // Find the index in path where word begins.
  int offset = 0;
  while (path_string(path).substr(offset, word.size()) != word) {
    ++offset;
  }

  int score = 0;
  for (int i = 0; i < word.size(); ++i) {
    char c = word[i];
    score += (letter_values_.at(c) * multiplier_at(path[i + offset]));
  }
  return std::ceil(score * (dict.IsCommonWord(word) ? 1.3 : 1));
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

std::string BongoGameState::path_string(const std::vector<Point> &path) const {
  std::string s = "";
  for (const Point &p : path) {
    s += char_at(p);
  }
  return s;
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

void BongoGameState::set_bonus_word_path(const std::vector<Point> &path) {
  bonus_word_path_ = path;
}

std::vector<Point> BongoGameState::bonus_word_path() const {
  return bonus_word_path_;
}

std::vector<Point> BongoGameState::row_path(int row) const {
  std::vector<Point> path;
  for (int col = 0; col < 5; ++col) {
    path.push_back({row, col});
  }
  return path;
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
