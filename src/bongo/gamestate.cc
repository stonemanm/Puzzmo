#include "gamestate.h"

#include <cctype>
#include <string>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"

namespace puzzmo::bongo {

namespace {

constexpr absl::string_view kLineWordLengthDifferenceError =
    "Word \"%s\" has length %d, but line has size %d.";
constexpr absl::string_view kLockedCellError =
    "The cell at %v is locked and cannot be altered.";
constexpr absl::string_view kNoCellAtPointError =
    "Point %v does not refer to a cell on the board.";

// A helper function to validate a `Point`.
bool HasCell(const Point &p) {
  return (0 <= p.row && p.row < 5 && 0 <= p.col && p.col < 5);
}

// A helper function to validate a row.
bool HasRow(int row) { return (0 <= row && row < 5); }

}  // namespace

// Constructors

Gamestate::Gamestate(const std::vector<std::string> board,
                     absl::flat_hash_map<char, int> letter_values,
                     LetterCount unplaced_letters,
                     std::vector<std::string> letter_board)
    : grid_(5, std::vector<Cell>(5)),
      letters_(unplaced_letters),
      unplaced_letters_(unplaced_letters),
      letter_values_(letter_values) {
  CHECK_EQ(board.size(), 5);
  CHECK_EQ(letter_board.size(), 5);

  for (int r = 0; r < 5; ++r) {
    CHECK_EQ(board[r].size(), 5);
    CHECK_EQ(letter_board[r].size(), 5);

    for (int c = 0; c < 5; ++c) {
      Cell &cell = grid_[r][c];
      cell.letter = letter_board[r][c];
      if (std::isalpha(cell.letter))
        (void)unplaced_letters_.RemoveLetter(cell.letter);

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

// Accessors

std::vector<Point> Gamestate::MultiplierPoints() const {
  std::vector<Point> multiplier_points;
  for (int r = 0; r < 5; ++r) {
    for (int c = 0; c < 5; ++c) {
      if (grid_[r][c].multiplier >= 2)
        multiplier_points.push_back({.row = r, .col = c});
    }
  }
  std::sort(multiplier_points.begin(), multiplier_points.end(),
            [*this](const Point &lhs, const Point &rhs) {
              const int lhs_mult = grid_[lhs.row][lhs.col].multiplier;
              const int rhs_mult = grid_[rhs.row][rhs.col].multiplier;
              return lhs_mult != rhs_mult ? lhs_mult > rhs_mult
                     : lhs.row != rhs.row ? lhs.row < rhs.row
                                          : lhs.col < rhs.col;
            });
  return multiplier_points;
}

std::vector<Point> Gamestate::DoublePoints() const {
  std::vector<Point> double_points;
  for (int r = 0; r < 5; ++r) {
    for (int c = 0; c < 5; ++c) {
      if (grid_[r][c].multiplier == 2)
        double_points.push_back({.row = r, .col = c});
    }
  }
  return double_points;
}

Point Gamestate::TriplePoint() const {
  for (int r = 0; r < 5; ++r) {
    for (int c = 0; c < 5; ++c) {
      if (grid_[r][c].multiplier == 3) return {.row = r, .col = c};
    }
  }
  return {-1, -1};  // Should never happen.
}

std::string Gamestate::NMostValuableLetters(int n) const {
  if (n <= 0) return "";

  std::string letters = unplaced_letters_.CharsInOrder();
  std::sort(letters.begin(), letters.end(), [this](char l, char r) {
    return letter_values_.at(l) > letter_values_.at(r);
  });
  return letters.substr(0, n);
}

std::vector<Point> Gamestate::line(int row) const {
  CHECK(HasRow(row));
  std::vector<Point> line;
  for (int col = 0; col < 5; ++col) line.push_back({.row = row, .col = col});
  return line;
}

std::string Gamestate::LineRegex(const std::vector<Point> &line) const {
  // TODO: Add the ability to search for 4 letter words.

  std::string s = LineString(line);
  if (LetterCount(s).empty()) return "";

  std::string rgx = "";
  for (char l : s) {
    absl::StrAppend(&rgx, std::isalpha(l)
                              ? std::string(1, l)
                              : unplaced_letters_.RegexMatchingContents());
  }
  return rgx;
}

std::string Gamestate::LineString(const std::vector<Point> &line) const {
  std::string s = "";
  for (const Point &p : line) s.push_back(grid_[p.row][p.col].letter);
  return s;
}

// Mutators

absl::Status Gamestate::ClearCell(const Point &p) {
  if (!HasCell(p))
    return absl::InvalidArgumentError(absl::StrFormat(kNoCellAtPointError, p));
  if (grid_[p.row][p.col].is_locked)
    return absl::FailedPreconditionError(absl::StrFormat(kLockedCellError, p));

  // If a letter is already in the cell, add it to `unplaced_letters_`.
  char letter_in_cell = grid_[p.row][p.col].letter;
  if (std::isalpha(letter_in_cell)) {
    if (absl::Status s = unplaced_letters_.AddLetter(letter_in_cell); !s.ok()) {
      LOG(ERROR) << s;
      return s;
    }
  }

  grid_[p.row][p.col].letter = kEmptyCell;
  return absl::OkStatus();
}

absl::Status Gamestate::FillCell(const Point &p, char l) {
  // Ensure we have that letter to place.
  if (absl::Status s = unplaced_letters_.RemoveLetter(l); !s.ok()) {
    LOG(ERROR) << s;
    return s;
  }
  if (absl::Status s = ClearCell(p); !s.ok()) {
    LOG(ERROR) << s;
    return s;
  }

  grid_[p.row][p.col].letter = l;
  return absl::OkStatus();
}

absl::Status Gamestate::ClearLine(const std::vector<Point> &line) {
  for (const Point &p : line) {
    if (!HasCell(p))
      return absl::InvalidArgumentError(
          absl::StrFormat(kNoCellAtPointError, p));

    // If the cell isn't locked and contains a letter, clear it.
    if (!grid_[p.row][p.col].is_locked &&
        std::isalpha(grid_[p.row][p.col].letter)) {
      if (absl::Status s = ClearCell(p); !s.ok()) {
        LOG(ERROR) << s;
        return s;
      }
    }
  }
  return absl::OkStatus();
}

absl::Status Gamestate::FillLine(const std::vector<Point> &line,
                                 absl::string_view word) {
  if (word.length() != line.size())
    return absl::InvalidArgumentError(absl::StrFormat(
        kLineWordLengthDifferenceError, word, word.length(), line.size()));

  for (int i = 0; i < line.size(); ++i) {
    const Point p = line[i];
    // If the letter in the cell already aligns with `word`, we don't call
    // `FillCell()`.
    if (grid_[p.row][p.col].letter == word[i]) continue;
    if (absl::Status s = FillCell(p, word[i]); !s.ok()) {
      LOG(ERROR) << s;
      return s;
    }
  }
  return absl::OkStatus();
}

absl::Status Gamestate::ClearBoard() {
  for (int r = 0; r < 5; ++r) {
    for (int c = 0; c < 5; ++c) {
      if (grid_[r][c].is_locked) continue;
      if (absl::Status s = ClearCell({r, c}); !s.ok()) {
        LOG(ERROR) << s;
        return s;
      }
    }
  }
  return absl::OkStatus();
}

// Relational

bool Gamestate::IsChildOf(const Gamestate &other) const {
  if (letters() != other.letters() || letter_values_ != other.letter_values() ||
      bonus_line_ != other.bonus_line())
    return false;
  for (int r = 0; r < 5; ++r) {
    for (int c = 0; c < 5; ++c) {
      if (grid_[r][c].multiplier != other.grid()[r][c].multiplier) return false;
      char l = other.grid()[r][c].letter;
      if (std::isalpha(l) && grid_[r][c].letter != l) return false;
    }
  }
  return true;
}

// Scoring

std::vector<std::vector<Point>> Gamestate::LinesToScore() const {
  std::vector<std::vector<Point>> lines;
  lines.push_back(line(0));
  lines.push_back(line(1));
  lines.push_back(line(2));
  lines.push_back(line(3));
  lines.push_back(line(4));
  lines.push_back(bonus_line_);
  return lines;
}

int Gamestate::UpperBoundOnScore() const {
  std::string tiles_in_value_order = NMostValuableLetters(25);
  int score = 0;
  score += letter_values_.at(tiles_in_value_order[0]) * 3;
  for (int i = 1; i < 8; ++i) {
    score += letter_values_.at(tiles_in_value_order[i]) * 2;
  }
  for (int i = 8; i < tiles_in_value_order.size(); ++i) {
    score += letter_values_.at(tiles_in_value_order[i]);
  }
  return std::ceil(1.3 * score);
}

// Operator

bool operator==(const Cell &lhs, const Cell &rhs) {
  return lhs.letter == rhs.letter && lhs.multiplier == rhs.multiplier;
}

bool operator==(const Gamestate &lhs, const Gamestate &rhs) {
  return lhs.grid() == rhs.grid() &&
         lhs.unplaced_letters() == rhs.unplaced_letters() &&
         lhs.letter_values() == rhs.letter_values() &&
         lhs.bonus_line() == rhs.bonus_line();
}

bool operator!=(const Cell &lhs, const Cell &rhs) { return !(lhs == rhs); }

bool operator!=(const Gamestate &lhs, const Gamestate &rhs) {
  return !(lhs == rhs);
}

}  // namespace puzzmo::bongo
