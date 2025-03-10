#include "gamestate.h"

#include <cctype>
#include <climits>
#include <string>

#include "absl/log/check.h"
#include "absl/strings/str_cat.h"

namespace puzzmo::bongo {

namespace {

bool HasCell(const Point &p) {
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

bool operator==(const Cell &lhs, const Cell &rhs) {
  return lhs.letter == rhs.letter && lhs.multiplier == rhs.multiplier;
}

bool operator!=(const Cell &lhs, const Cell &rhs) { return !(lhs == rhs); }

Gamestate::Gamestate(const std::vector<std::string> board,
                     absl::flat_hash_map<char, int> letter_values,
                     LetterCount letter_pool,
                     std::vector<std::string> letter_board)
    : grid_(5, std::vector<Cell>(5)),
      letter_pool_(letter_pool),
      values_(letter_values) {
  CHECK_EQ(board.size(), 5);
  CHECK_EQ(letter_board.size(), 5);

  for (int r = 0; r < 5; ++r) {
    CHECK_EQ(board[r].size(), 5);
    CHECK_EQ(letter_board[r].size(), 5);

    for (int c = 0; c < 5; ++c) {
      Cell &cell = grid_[r][c];
      cell.letter = letter_board[r][c];
      switch (board[r][c]) {
        case kBonusCell:
          bonus_line_.push_back({.row = r, .col = c});
          break;
        case kDoubleMultiplier:
          cell.multiplier = 2;
          break;
        case kTripleMultiplier:
          cell.multiplier = 3;
          break;
        default:
          break;
      }
    }
  }
}

absl::Status Gamestate::ClearCell(const Point &p) {
  if (!HasCell(p))
    return absl::InvalidArgumentError(absl::StrCat(
        "Point ", p, " does not refer to a cell on the board.\n", *this));

  if (is_locked_at(p))
    return absl::FailedPreconditionError(
        absl::StrCat("Cell ", p, " is locked and cannot be altered.\n", *this));

  if (char b = char_at(p); std::isalpha(b)) {
    if (auto s = letter_pool_.AddLetter(b); !s.ok()) return s.status();
  }

  set_char_at(p, '_');
  return absl::OkStatus();
}

absl::Status Gamestate::FillCell(const Point &p, char c) {
  if (auto s = letter_pool_.RemoveLetter(c); !s.ok()) return s.status();

  if (auto s = ClearCell(p); !s.ok()) return s;

  set_char_at(p, c);
  return absl::OkStatus();
}

absl::Status Gamestate::ClearPath(const std::vector<Point> &path) {
  for (const Point &p : path) {
    if (!HasCell(p))
      return absl::InvalidArgumentError(absl::StrCat(
          "Point ", p, " does not refer to a cell on the board.\n", *this));
    if (is_locked_at(p) || !std::isalpha(char_at(p))) continue;
    if (auto s = ClearCell(p); !s.ok()) return s;
  }
  return absl::OkStatus();
}

absl::Status Gamestate::FillPath(const std::vector<Point> &path,
                                 absl::string_view sv) {
  if (path.size() != sv.length())
    return absl::InvalidArgumentError(
        absl::StrCat("Word has length ", sv.length(), " but path has size ",
                     path.size(), ".\n", *this));

  for (int i = 0; i < path.size(); ++i) {
    const Point p = path[i];
    if (char_at(path[i]) == sv[i]) continue;
    if (auto s = FillCell(p, sv[i]); !s.ok()) return s;
  }
  return absl::OkStatus();
}

absl::Status Gamestate::ClearBoard() {
  for (int row = 0; row < 5; ++row) {
    for (int col = 0; col < 5; ++col) {
      const Point p = {row, col};
      if (is_locked_at(row, col)) continue;
      if (auto s = ClearCell(p); !s.ok()) return s;
    }
  }
  return absl::OkStatus();
}

std::vector<std::vector<Point>> Gamestate::PathsToScore() const {
  std::vector<std::vector<Point>> paths;
  paths.push_back(row_path(0));
  paths.push_back(row_path(1));
  paths.push_back(row_path(2));
  paths.push_back(row_path(3));
  paths.push_back(row_path(4));
  paths.push_back(bonus_line_);
  return paths;
}

std::string Gamestate::GetWord(const std::vector<Point> &path) const {
  int threshold = (path == bonus_line_) ? 4 : 3;
  std::string path_substr = LongestAlphaSubstring(path_string(path));
  return (path_substr.length() >= threshold) ? path_substr : "";
}

bool Gamestate::IsChildOf(const Gamestate &other) const {
  if (AllLetters() != other.AllLetters() || values_ != other.values() ||
      bonus_line_ != other.bonus_path())
    return false;
  for (int r = 0; r < 5; ++r) {
    for (int c = 0; c < 5; ++c) {
      if (grid_[r][c].multiplier != other.grid()[r][c].multiplier) return false;
      char l = other.char_at(r, c);
      if (std::isalpha(l) && char_at(r, c) != l) return false;
    }
  }
  return true;
}

bool Gamestate::IsComplete() const {
  for (const auto &path : PathsToScore()) {
    if (path == bonus_line_) continue;
    if (GetWord(path).empty()) return false;
  }
  return true;
}

int Gamestate::MostRestrictedWordlessRow() const {
  int row_to_focus = 0;
  int most_letters = INT_MIN;
  for (int row = 0; row < 5; ++row) {
    if (!GetWord(row_path(row)).empty()) continue;
    int letters = absl::c_count_if(
        grid_[row], [](const Cell &cell) { return cell.letter != kEmptyCell; });
    if (letters > most_letters) {
      most_letters = letters;
      row_to_focus = row;
    }
  }
  return row_to_focus;
}

LetterCount Gamestate::AllLetters() const {
  LetterCount all = letter_pool_;
  for (const std::vector<Cell> &row : grid_) {
    for (const Cell &cell : row) {
      (void)all.AddLetter(cell.letter);
    }
  }
  return all;
}

int Gamestate::NumLetters() const {
  return NumLettersLeft() + NumLettersPlaced();
}

int Gamestate::NumLettersLeft() const { return letter_pool_.size(); }

int Gamestate::NumLettersPlaced() const {
  int count = 0;
  for (const std::vector<Cell> &row : grid_) {
    for (const Cell &cell : row) {
      if (cell.letter != kEmptyCell) ++count;
    }
  }
  return count;
}

std::vector<Point> Gamestate::MultiplierCells() const {
  std::vector<Point> multiplier_cells;
  for (int row = 0; row < 5; ++row) {
    for (int col = 0; col < 5; ++col) {
      if (multiplier_at({row, col}) >= 2) {
        multiplier_cells.push_back({row, col});
      }
    }
  }
  return multiplier_cells;
}

std::string Gamestate::NMostValuableTiles(int n) const {
  if (n <= 0) return "";

  std::string letters = letter_pool_.CharsInOrder();
  std::sort(letters.begin(), letters.end(),
            [this](char l, char r) { return values_.at(l) > values_.at(r); });
  return letters.substr(0, n);
}

std::string Gamestate::RegexForPath(const std::vector<Point> &path) const {
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

/** * * * * * * * * * * *
 * Accessors & mutators *
 * * * * * * * * * * * **/

std::vector<Point> Gamestate::row_path(int row) const {
  if (!HasRow(row)) return {};
  std::vector<Point> path;
  for (int col = 0; col < 5; ++col) {
    path.push_back({row, col});
  }
  return path;
}

std::string Gamestate::path_string(const std::vector<Point> &path) const {
  std::string s = "";
  for (const Point &p : path) {
    s += char_at(p);
  }
  return s;
}

char Gamestate::char_at(const Point &p) const {
  return HasCell(p) ? grid_[p.row][p.col].letter : '\0';
}

char Gamestate::char_at(int row, int col) const {
  return char_at({.row = row, .col = col});
}

void Gamestate::set_char_at(const Point &p, char c) {
  if (!HasCell(p)) return;
  grid_[p.row][p.col].letter = c;
}

void Gamestate::set_char_at(int row, int col, char c) {
  set_char_at({.row = row, .col = col}, c);
}

int Gamestate::multiplier_at(const Point &p) const {
  if (!HasCell(p)) return 0;
  return grid_[p.row][p.col].multiplier;
}

int Gamestate::multiplier_at(int row, int col) const {
  return multiplier_at({.row = row, .col = col});
}

bool Gamestate::is_locked_at(const Point &p) const {
  if (!HasCell(p)) return false;
  return grid_[p.row][p.col].is_locked;
}

bool Gamestate::is_locked_at(int row, int col) const {
  return is_locked_at({.row = row, .col = col});
}

void Gamestate::set_is_locked_at(const Point &p, bool is_locked) {
  if (!HasCell(p)) return;
  grid_[p.row][p.col].is_locked = is_locked;
}

void Gamestate::set_is_locked_at(int row, int col, bool is_locked) {
  set_is_locked_at({.row = row, .col = col}, is_locked);
}

/** * * * * * * * * * * *
 * Overloaded operators *
 * * * * * * * * * * * **/

bool operator==(const Gamestate &lhs, const Gamestate &rhs) {
  return lhs.grid() == rhs.grid() && lhs.letter_pool() == rhs.letter_pool() &&
         lhs.values() == rhs.values() && lhs.bonus_path() == rhs.bonus_path();
}

bool operator!=(const Gamestate &lhs, const Gamestate &rhs) {
  return !(lhs == rhs);
}

}  // namespace puzzmo::bongo
