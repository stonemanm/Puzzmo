#include "bongo_gamestate.h"

#include <cctype>
#include <climits>
#include <cmath>
#include <string>

#include "absl/strings/str_cat.h"

namespace puzzmo {

constexpr char kBonusSpace = '*';
constexpr char kDoubleMultiplier = '2';
constexpr char kTripleMultiplier = '3';

namespace {

bool HasSquare(const Point &p) {
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
                               LetterCount letter_pool,
                               std::vector<std::string> letter_board)
    : letter_pool_(letter_pool),
      letter_board_(std::vector<std::string>(5, std::string(5, '_'))),
      mult_board_(5, std::vector<int>(5)),
      lock_board_(5, std::vector<bool>(5)),
      values_(letter_values) {
  // Parse the inputs one space at a time.
  for (int row = 0; row < 5; ++row) {
    for (int col = 0; col < 5; ++col) {
      if (row < letter_board.size() && col < letter_board[row].size())
        set_char_at(row, col, letter_board[row][col]);
      switch (board[row][col]) {
        case kBonusSpace:
          bonus_path_.push_back({row, col});
          mult_board_[row][col] = 1;
          break;
        case kDoubleMultiplier:
          mult_board_[row][col] = 2;
          break;
        case kTripleMultiplier:
          mult_board_[row][col] = 3;
          break;
        default:
          mult_board_[row][col] = 1;
          break;
      }
    }
  }
}

absl::Status BongoGameState::ClearSquare(const Point &p) {
  if (!HasSquare(p))
    return absl::InvalidArgumentError(absl::StrCat(
        "Point ", p, " does not refer to a square on the board.\n", *this));

  if (is_locked_at(p))
    return absl::FailedPreconditionError(absl::StrCat(
        "Square ", p, " is locked and cannot be altered.\n", *this));

  if (char b = char_at(p); std::isalpha(b)) {
    if (auto s = letter_pool_.AddLetter(b); !s.ok()) return s.status();
  }

  set_char_at(p, '_');
  return absl::OkStatus();
}

absl::Status BongoGameState::FillSquare(const Point &p, char c) {
  if (auto s = letter_pool_.RemoveLetter(c); !s.ok()) return s.status();

  if (auto s = ClearSquare(p); !s.ok()) return s;

  set_char_at(p, c);
  return absl::OkStatus();
}

absl::Status BongoGameState::ClearPath(const std::vector<Point> &path) {
  for (const Point &p : path) {
    if (!HasSquare(p))
      return absl::InvalidArgumentError(absl::StrCat(
          "Point ", p, " does not refer to a square on the board.\n", *this));
    if (is_locked_at(p) || !std::isalpha(char_at(p))) continue;
    if (auto s = ClearSquare(p); !s.ok()) return s;
  }
  return absl::OkStatus();
}

absl::Status BongoGameState::FillPath(const std::vector<Point> &path,
                                      absl::string_view sv) {
  if (path.size() != sv.length())
    return absl::InvalidArgumentError(
        absl::StrCat("Word has length ", sv.length(), " but path has size ",
                     path.size(), ".\n", *this));

  for (int i = 0; i < path.size(); ++i) {
    const Point p = path[i];
    if (char_at(path[i]) == sv[i]) continue;
    if (auto s = FillSquare(p, sv[i]); !s.ok()) return s;
  }
  return absl::OkStatus();
}

absl::Status BongoGameState::ClearBoard() {
  for (int row = 0; row < 5; ++row) {
    for (int col = 0; col < 5; ++col) {
      const Point p = {row, col};
      if (is_locked_at(row, col)) continue;
      if (auto s = ClearSquare(p); !s.ok()) return s;
    }
  }
  return absl::OkStatus();
}

std::string BongoGameState::GetWord(const std::vector<Point> &path) const {
  int threshold = (path == bonus_path_) ? 4 : 3;
  std::string path_substr = LongestAlphaSubstring(path_string(path));
  return (path_substr.length() >= threshold) ? path_substr : "";
}

bool BongoGameState::IsComplete() const {
  for (int row = 0; row < 5; ++row) {
    if (GetWord(row_path(row)).empty()) return false;
  }
  return true;
}

int BongoGameState::MostRestrictedWordlessRow() const {
  int row_to_focus = 0;
  int most_letters = INT_MIN;
  for (int row = 0; row < 5; ++row) {
    if (!GetWord(row_path(row)).empty()) continue;
    int letters = LetterCount(letter_board_[row]).size();
    if (letters > most_letters) {
      most_letters = letters;
      row_to_focus = row;
    }
  }
  return row_to_focus;
}

std::vector<Point> BongoGameState::MultiplierSquares() const {
  std::vector<Point> mult_squares;
  for (int row = 0; row < 5; ++row) {
    for (int col = 0; col < 5; ++col) {
      if (multiplier_at({row, col}) >= 2) {
        mult_squares.push_back({row, col});
      }
    }
  }
  return mult_squares;
}

std::string BongoGameState::NMostValuableTiles(int n) const {
  if (n <= 0) return "";

  std::string letters = letter_pool_.CharsInOrder();
  std::sort(letters.begin(), letters.end(),
            [this](char l, char r) { return values_.at(l) > values_.at(r); });
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
                              : letter_pool_.RegexMatchingContents());
  }
  return rgx;
}

int BongoGameState::CalculateScore(const BongoDictionary &dict) const {
  return CalculatePathScore(row_path(0), dict) +
         CalculatePathScore(row_path(1), dict) +
         CalculatePathScore(row_path(2), dict) +
         CalculatePathScore(row_path(3), dict) +
         CalculatePathScore(row_path(4), dict) +
         CalculatePathScore(bonus_path(), dict);
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
    score += (values_.at(c) * multiplier_at(path[i + offset]));
  }
  return std::ceil(score * (dict.IsCommonWord(word) ? 1.3 : 1));
}

/** * * * * * * * * * * *
 * Accessors & mutators *
 * * * * * * * * * * * **/

std::vector<Point> BongoGameState::row_path(int row) const {
  if (!HasRow(row)) return {};
  std::vector<Point> path;
  for (int col = 0; col < 5; ++col) {
    path.push_back({row, col});
  }
  return path;
}

std::string BongoGameState::path_string(const std::vector<Point> &path) const {
  std::string s = "";
  for (const Point &p : path) {
    s += char_at(p);
  }
  return s;
}

char BongoGameState::char_at(const Point &p) const {
  return HasSquare(p) ? letter_board_[p.row][p.col] : '\0';
}

char BongoGameState::char_at(int row, int col) const {
  return char_at({row, col});
}

void BongoGameState::set_char_at(const Point &p, char c) {
  if (!HasSquare(p)) return;
  letter_board_[p.row][p.col] = c;
}

void BongoGameState::set_char_at(int row, int col, char c) {
  set_char_at({row, col}, c);
}

int BongoGameState::multiplier_at(const Point &p) const {
  if (!HasSquare(p)) return 0;
  return mult_board_[p.row][p.col];
}

int BongoGameState::multiplier_at(int row, int col) const {
  return multiplier_at({row, col});
}

bool BongoGameState::is_locked_at(const Point &p) const {
  if (!HasSquare(p)) return false;
  return lock_board_[p.row][p.col];
}

bool BongoGameState::is_locked_at(int row, int col) const {
  return is_locked_at({row, col});
}

void BongoGameState::set_is_locked_at(const Point &p, bool is_locked) {
  if (!HasSquare(p)) return;
  lock_board_[p.row][p.col] = is_locked;
}

void BongoGameState::set_is_locked_at(int row, int col, bool is_locked) {
  set_is_locked_at({row, col}, is_locked);
}

/** * * * * * * * * * * *
 * Overloaded operators *
 * * * * * * * * * * * **/

bool operator==(const BongoGameState &lhs, const BongoGameState &rhs) {
  return lhs.letter_pool() == rhs.letter_pool() &&
         lhs.letter_board() == rhs.letter_board() &&
         lhs.mult_board() == rhs.mult_board() &&
         lhs.lock_board() == rhs.lock_board() && lhs.values() == rhs.values() &&
         lhs.bonus_path() == rhs.bonus_path();
}

bool operator!=(const BongoGameState &lhs, const BongoGameState &rhs) {
  return !(lhs == rhs);
}

}  // namespace puzzmo
