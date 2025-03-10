#include "solver.h"

#include <climits>
#include <string>

#include "absl/container/flat_hash_set.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"

namespace puzzmo::bongo {
namespace {

// A helper function to identify the longest substring of alphabetical
// characters in `s`.
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

// Constructors

Solver::Solver(const Dict &dict, const Gamestate &state, Parameters params)
    : dict_(dict),
      lines_(state.LinesToScore()),
      starting_state_(state),
      state_(state),
      best_state_(state),
      tiles_for_bonus_words_(params.tiles_for_bonus_words),
      tiles_for_multiplier_tiles_(params.tiles_for_multiplier_tiles) {}

// Mutators

void Solver::reset() { state_ = starting_state_; }

absl::StatusOr<Gamestate> Solver::Solve() {
  std::vector<Point> bonus_line = state_.bonus_line();
  std::vector<Point> multiplier_cells = state_.MultiplierCells();

  // To narrow the search space, grab the most valuable tiles and try to
  // make bonus words using three of them.
  absl::flat_hash_set<std::string> bonus_words_to_try;

  // If there are already letters in the bonus line, adjust this phase
  // accordingly
  LetterCount bplc(state_.LineString(state_.bonus_line()));
  LetterCount valuable_letters(
      state_.NMostValuableLetters(tiles_for_bonus_words_));
  absl::flat_hash_set<std::string> combos =
      valuable_letters.CombinationsOfSize(3 - bplc.size());
  for (const auto &combo : combos) {
    auto words = dict_.WordsMatchingParameters(
        {.min_length = 4,
         .max_length = 4,
         .min_letters = LetterCount(combo),
         .max_letters = state_.unplaced_letters() + bplc,
         .matching_regex = state_.LineRegex(bonus_line)});
    bonus_words_to_try.insert(words.begin(), words.end());
  }
  LOG(INFO) << "Found " << bonus_words_to_try.size() << " bonus words to try.";
  LOG(INFO) << absl::StrJoin(bonus_words_to_try, ", ");

  int loops = 0;
  for (const std::string &bonus_word : bonus_words_to_try) {
    LOG(INFO) << "Beginning loop " << ++loops << "/"
              << bonus_words_to_try.size() << " with bonus word \""
              << bonus_word << "\"";

    // Place the bonus word on the board
    if (absl::Status s = state_.FillLine(bonus_line, bonus_word); !s.ok())
      return s;
    for (const Point &p : bonus_line) state_[p].is_locked = true;

    // Grab the high-scoring tiles not used by the bonus word, and try all the
    // permutations of them on the open multiplier_squares.
    std::string mvls = state_.NMostValuableLetters(tiles_for_multiplier_tiles_);
    std::sort(mvls.begin(), mvls.end());
    do {
      std::vector<Point> locked_here;
      for (int i = 0; i < 3; ++i) {
        const Point p = multiplier_cells[i];
        Cell &cell = state_[p];
        if (std::isalpha(cell.letter)) continue;
        // If we have an error placing tiles, clean up the ones we have placed
        // so far and then move on.
        if (absl::Status s = state_.FillCell(p, mvls[i]); !s.ok()) return s;

        cell.is_locked = true;
        locked_here.push_back(p);
      }

      if (absl::Status s = FindWordsRecursively(); !s.ok()) return s;

      for (int i = 0; i < locked_here.size(); ++i) {
        state_[locked_here[i]].is_locked = false;
        if (absl::Status s = state_.ClearCell(locked_here[i]); !s.ok())
          return s;
      }
    } while (std::next_permutation(mvls.begin(), mvls.end()));
    for (const Point &p : bonus_line) state_[p].is_locked = false;
  }
  if (best_score_ == 0) {
    ++tiles_for_bonus_words_;
    ++tiles_for_multiplier_tiles_;
    LOG(INFO) << "No solutions found. Trying again with a broader "
                 "search.";
    return Solve();
  }
  return best_state_;
}

absl::Status Solver::FindWordsRecursively() {
  // Check for failure (error, really)
  if (state_.letters().size() < 25)
    return absl::UnknownError("You've met with a terrible fate, haven't you?");

  // Check for success.
  if (state_.IsComplete()) {
    if (int score = Score(); score > best_score_) {
      VLOG(1) << absl::StrCat("New best score! (", score, ")");
      for (const std::vector<Point> &line : lines_) {
        std::string word = state_.GetWord(line);
        VLOG(2) << absl::StrCat(LineScore(line), " - ", word,
                                (dict_.IsCommonWord(word) ? " is" : " isn't"),
                                " a common word.");
      }
      VLOG(1) << state_;
      best_state_ = state_;
      best_score_ = score;
    }
    return absl::OkStatus();
  }

  // Focus on the next row that needs filling, and find all possible options.
  int row = MostRestrictedWordlessRow();
  std::vector<Point> line = state_.line(row);
  LetterCount line_letters(state_.LineString(line));
  absl::flat_hash_set<std::string> matches = dict_.WordsMatchingParameters(
      {.min_length = 5,  // TODO: allow shorter words
       .max_length = 5,
       .min_letters = line_letters,
       .max_letters = line_letters + state_.unplaced_letters(),
       .matching_regex = state_.LineRegex(line)});

  // Try all possible options, recurse, then backtrack. If something goes wrong,
  // return the error.
  for (const absl::string_view word : matches) {
    if (absl::Status s = state_.FillLine(line, word); !s.ok()) return s;
    if (absl::Status s = FindWordsRecursively(); !s.ok()) return s;
    if (absl::Status s = state_.ClearLine(line); !s.ok()) return s;
  }
  return absl::OkStatus();
}

// Score

int Solver::LineScore(const std::vector<Point> &line) const {
  std::string word = state_.GetWord(line);
  if (!dict_.contains(word)) return 0;

  // Find the index in line where word begins.
  int offset = 0;
  while (state_.LineString(line).substr(offset, word.size()) != word) ++offset;

  int score = 0;
  for (int i = 0; i < word.size(); ++i) {
    char c = word[i];
    score += state_.letter_values().at(c) * state_[line[i + offset]].multiplier;
  }
  return std::ceil(score * (dict_.IsCommonWord(word) ? 1.3 : 1));
}

int Solver::Score() const {
  int score = 0;
  for (const std::vector<Point> &line : lines_) score += LineScore(line);
  return score;
}

// Words

std::string Solver::GetWord(const std::vector<Point> &line) const {
  int threshold = (line != state_.bonus_line()) ? 3 : 4;
  std::string line_substr = LongestAlphaSubstring(state_.LineString(line));
  return (line_substr.length() < threshold) ? "" : line_substr;
}

bool Solver::IsComplete() const {
  return std::all_of(lines_.begin(), lines_.end(),
                     [*this](const std::vector<Point> &line) {
                       return !GetWord(line).empty();
                     });
}

int Solver::MostRestrictedWordlessRow() const {
  int most_letters_placed = INT_MIN;
  int row_to_focus = 0;

  for (int row = 0; row < 5; ++row) {
    if (!GetWord(lines_[row]).empty()) continue;
    int letters = absl::c_count_if(state_.grid()[row], [](const Cell &cell) {
      return cell.letter != kEmptyCell;
    });
    if (letters > most_letters_placed) {
      most_letters_placed = letters;
      row_to_focus = row;
    }
  }
  return row_to_focus;
}

}  // namespace puzzmo::bongo