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
      bonus_line_(state.bonus_line()),
      multiplier_points_(state.MultiplierPoints()),
      starting_state_(state),
      best_state_(state),
      tiles_for_bonus_words_(params.tiles_for_bonus_words),
      tiles_for_multiplier_tiles_(params.tiles_for_multiplier_tiles) {}

// Mutators

absl::Status Solver::FillLine(const std::vector<Point> &line,
                              absl::string_view word) {
  Gamestate state = CurrentState();
  if (absl::Status s = state.FillLine(line, word); !s.ok()) {
    LOG(ERROR) << s;
    return s;
  }
  steps_.push_back(state);
  return absl::OkStatus();
}

absl::Status Solver::FillMultiplierCells(absl::string_view letters) {
  Gamestate state = CurrentState();
  int l = 0;
  for (int m = 0; m < multiplier_points_.size() && l < letters.length(); ++m) {
    const Point p = multiplier_points_[m];
    if (state[p].letter != kEmptyCell) continue;
    if (absl::Status s = state.FillCell(p, letters[l++]); !s.ok()) {
      LOG(ERROR) << s;
      return s;
    }
  }
  steps_.push_back(state);
  return absl::OkStatus();
}

absl::StatusOr<Gamestate> Solver::Solve() {
  // To narrow the search space, grab the most valuable tiles and try to
  // make bonus words using three of them.
  absl::flat_hash_set<std::string> bonus_words_to_try;

  // If there are already letters in the bonus line, adjust this phase
  // accordingly
  LetterCount bplc(starting_state_.LineString(bonus_line_));
  LetterCount valuable_letters(
      starting_state_.NMostValuableLetters(tiles_for_bonus_words_));
  absl::flat_hash_set<std::string> combos =
      valuable_letters.CombinationsOfSize(3 - bplc.size());
  for (const std::string &combo : combos) {
    absl::flat_hash_set<std::string> words = dict_.WordsMatchingParameters(
        {.min_length = 4,
         .max_length = 4,
         .min_letters = LetterCount(combo),
         .max_letters = starting_state_.unplaced_letters() + bplc,
         .matching_regex = starting_state_.LineRegex(bonus_line_)});
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
    if (absl::Status s = FillLine(bonus_line_, bonus_word); !s.ok()) {
      LOG(ERROR) << s;
      return s;
    }

    // Grab the high-scoring tiles not used by the bonus word, and try all the
    // permutations of them on the open multiplier_squares.
    std::string mvls =
        CurrentState().NMostValuableLetters(tiles_for_multiplier_tiles_);
    std::sort(mvls.begin(), mvls.end());
    do {
      if (absl::Status s = FillMultiplierCells(mvls); !s.ok()) {
        LOG(ERROR) << s;
        return s;
      }
      if (absl::Status s = FindWordsRecursively(); !s.ok()) {
        LOG(ERROR) << s;
        return s;
      }
      steps_.pop_back();  // The multiplier tiles

    } while (std::next_permutation(mvls.begin(), mvls.end()));
    steps_.pop_back();  // The bonus word
  }

  // Loop again if nothing found.
  if (best_score_ == 0) {
    ++tiles_for_bonus_words_;
    ++tiles_for_multiplier_tiles_;
    LOG(INFO) << "No solutions found. Trying again with a broader search.";
    return Solve();
  }

  return best_state_;
}

absl::Status Solver::FindWordsRecursively() {
  Gamestate state = CurrentState();
  // Check for failure (error, really)
  if (state.letters().size() < 25)
    return absl::UnknownError("You've met with a terrible fate, haven't you?");

  // Check for success.
  if (IsComplete()) {
    UpdateBestState();
    return absl::OkStatus();
  }

  // Focus on the next row that needs filling, and find all possible options.
  int row = MostRestrictedWordlessRow();
  std::vector<Point> line = state.line(row);
  LetterCount line_letters(state.LineString(line));
  absl::flat_hash_set<std::string> matches = dict_.WordsMatchingParameters(
      {.min_length = 5,  // TODO: allow shorter words
       .max_length = 5,
       .min_letters = line_letters,
       .max_letters = line_letters + state.unplaced_letters(),
       .matching_regex = state.LineRegex(line)});

  // Try all possible options, recurse, then backtrack. If something goes wrong,
  // return the error.
  for (const absl::string_view word : matches) {
    if (absl::Status s = FillLine(line, word); !s.ok()) {
      LOG(ERROR) << s;
      return s;
    }
    if (absl::Status s = FindWordsRecursively(); !s.ok()) {
      LOG(ERROR) << s;
      return s;
    }
    steps_.pop_back();
  }
  return absl::OkStatus();
}

// Score

int Solver::LineScore(const std::vector<Point> &line) const {
  const Gamestate state = CurrentState();
  const std::string word = GetWord(line);
  if (!dict_.contains(word)) return 0;

  // Find the index in line where word begins.
  int offset = 0;
  while (state.LineString(line).substr(offset, word.size()) != word) ++offset;

  int score = 0;
  for (int i = 0; i < word.size(); ++i) {
    char c = word[i];
    score += state.letter_values().at(c) * state[line[i + offset]].multiplier;
  }
  return std::ceil(score * (dict_.IsCommonWord(word) ? 1.3 : 1));
}

int Solver::Score() const {
  int score = 0;
  for (const std::vector<Point> &line : lines_) score += LineScore(line);
  return score;
}

void Solver::UpdateBestState() {
  if (int score = Score(); score > best_score_) {
    best_score_ = score;
    best_state_ = CurrentState();
    LOG(INFO) << absl::StrCat("New best score! (", best_score_, ")");
    for (const std::vector<Point> &line : lines_) {
      std::string word = GetWord(line);
      LOG(INFO) << absl::StrCat(LineScore(line), " - ", word,
                                (dict_.IsCommonWord(word) ? " is" : " isn't"),
                                " a common word.");
    }
    LOG(INFO) << best_state_;
  }
}

// Words

std::string Solver::GetWord(const std::vector<Point> &line) const {
  const int threshold = (line == bonus_line_) ? 4 : 3;
  const std::string word =
      LongestAlphaSubstring(CurrentState().LineString(line));
  return (word.length() >= threshold && dict_.contains(word)) ? word : "";
}

bool Solver::IsComplete() const {
  return std::none_of(lines_.begin(), lines_.end(),
                      [*this](const std::vector<Point> &line) {
                        return GetWord(line).empty();
                      });
}

int Solver::MostRestrictedWordlessRow() const {
  int most_letters_placed = INT_MIN;
  int row_to_focus = 0;
  for (int row = 0; row < 5; ++row) {
    if (!(GetWord(lines_[row]).empty())) continue;
    const int letters = absl::c_count_if(
        CurrentState().grid()[row],
        [](const Cell &cell) { return cell.letter != kEmptyCell; });
    if (letters > most_letters_placed) {
      most_letters_placed = letters;
      row_to_focus = row;
    }
  }
  return row_to_focus;
}

}  // namespace puzzmo::bongo