#include "solver.h"

#include <climits>
#include <string>

#include "absl/container/flat_hash_set.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"

namespace puzzmo::bongo {
namespace {

constexpr absl::string_view kVerboseLoop =
    "%sBeginning loop %d/%d with %s \"%s\".";

// How to fill the second "%s" in `kVerboseLoop`.
std::string VerboseLoopText(const Technique &t) {
  switch (t) {
    case Technique::kFillMostRestrictedRow:
      return "word";
    case Technique::kFillBonusWordCells:
      return "bonus word";
    case Technique::kFillMultiplierCells:
      return "letters";
  }
}

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

/** * * * * * * *
 * Constructors *
 * * * * * * * **/

Solver::Solver(const Dict &dict, const Gamestate &state, Parameters params)
    : dict_(dict),
      lines_(state.LinesToScore()),
      bonus_line_(state.bonus_line()),
      multiplier_points_(state.MultiplierPoints()),
      starting_state_(state),
      best_state_(state),
      state_(state),
      params_(params) {}

/** * * * * *
 * Mutators *
 * * * * * **/

void Solver::reset() {
  state_ = starting_state_;
  locks_.clear();
}

absl::StatusOr<Gamestate> Solver::Solve() {
  if (absl::Status s = RecursiveHelper(0); !s.ok()) {
    LOG(ERROR) << s;
    return s;
  }
  // Loop again if nothing found.
  if (best_score_ == 0) {
    ++params_.num_tiles_for_bonus_words;
    ++params_.num_tiles_for_mult_cells;
    LOG(INFO) << "No solutions found. Trying again with a broader search.";
    return Solve();
  }
  return best_state_;
}

absl::Status Solver::RecursiveHelper(int i) {
  // Check for success/failure.
  if (IsComplete()) {
    UpdateBestState();
    return absl::OkStatus();
  }

  // Get the technique to use.
  Technique t = (i < params_.techniques.size())
                    ? params_.techniques[i]
                    : Technique::kFillMostRestrictedRow;

  // Get the cells targeted by the technique and the options for them.
  std::vector<Point> cells;
  absl::flat_hash_set<std::string> options;
  switch (t) {
    case Technique::kFillMostRestrictedRow:
      cells = state_.line(MostRestrictedWordlessRow());
      options = OptionsForLine(cells);
      break;

    case Technique::kFillBonusWordCells:
      cells = bonus_line_;
      options = OptionsForBonusWord();
      break;

    case Technique::kFillMultiplierCells:
      cells = RemainingMultiplierCells();
      options = OptionsForMultiplierTiles();
      break;
  }

  int loop = 0;
  for (const absl::string_view letters : options) {
    if (i < 3)
      LOG(INFO) << absl::StrFormat(kVerboseLoop, std::string(i + 1, ' '),
                                   ++loop, options.size(), VerboseLoopText(t),
                                   letters);

    // Place the letters in the cells.
    if (absl::Status s = FillCells(cells, letters); !s.ok()) {
      LOG(ERROR) << s;
      return s;
    }
    // Recurse.
    if (absl::Status s = RecursiveHelper(i + 1); !s.ok()) {
      LOG(ERROR) << s;
      return s;
    }
    // Undo the placement.
    if (absl::Status s = ClearCells(); !s.ok()) {
      LOG(ERROR) << s;
      return s;
    }
  }
  return absl::OkStatus();
}

std::vector<Point> Solver::RemainingMultiplierCells() const {
  std::vector<Point> open_multiplier_points;
  for (const Point &p : multiplier_points_)
    if (state_[p].letter == kEmptyCell) open_multiplier_points.push_back(p);
  return open_multiplier_points;
}

absl::Status Solver::FillCells(const std::vector<Point> &cells,
                               const absl::string_view letters) {
  if (absl::Status s = state_.FillLine(cells, letters); !s.ok()) {
    LOG(ERROR) << s;
    return s;
  }

  locks_.push_back({});
  for (const Point &p : cells) {
    if (state_[p].is_locked) continue;
    locks_.back().insert(p);
    state_[p].is_locked = true;
  }
  return absl::OkStatus();
}

absl::Status Solver::ClearCells() {
  absl::flat_hash_set<Point> locks = locks_.back();
  locks_.pop_back();
  for (const Point &p : locks) {
    state_[p].is_locked = false;
    if (absl::Status s = state_.ClearCell(p); !s.ok()) {
      LOG(ERROR) << s;
      return s;
    }
  }
  return absl::OkStatus();
}

/** * * * **
 * Options *
 ** * * * **/

absl::flat_hash_set<std::string> Solver::OptionsForBonusWord() const {
  const LetterCount line_contents(state_.LineString(bonus_line_));
  Dict::SearchParameters params = {
      .min_length = 4,
      .max_length = 4,
      .max_letters = state_.unplaced_letters() + line_contents,
      .matching_regex = state_.LineRegex(bonus_line_)};

  // We narrow the possible bonus words by requiring they use a certain number
  // of the most valuable tiles.
  const LetterCount top_letters(
      state_.NMostValuableLetters(params_.num_tiles_for_bonus_words));
  absl::flat_hash_set<std::string> combos =
      top_letters.CombinationsOfSize(3 - line_contents.size());

  // For each combo, get the possible words and add them to the set.
  absl::flat_hash_set<std::string> options;
  for (const absl::string_view combo : combos) {
    params.min_letters = LetterCount(combo);
    absl::flat_hash_set<std::string> words =
        dict_.WordsMatchingParameters(params);
    options.insert(words.begin(), words.end());
  }
  return options;
}

absl::flat_hash_set<std::string> Solver::OptionsForLine(
    const std::vector<Point> &line) const {
  const LetterCount line_contents(state_.LineString(line));
  const int n = line.size();
  return dict_.WordsMatchingParameters(
      {.min_length = n,  // TODO: 3
       .max_length = n,
       .min_letters = line_contents,
       .max_letters = line_contents + state_.unplaced_letters(),
       .matching_regex = state_.LineRegex(line)});
}

absl::flat_hash_set<std::string> Solver::OptionsForMultiplierTiles() const {
  // We only consider a certain number of high-value letters to place on the
  // multiplier tiles.
  const LetterCount top_letters(
      state_.NMostValuableLetters(params_.num_tiles_for_mult_cells));
  const int k = absl::c_count_if(multiplier_points_, [*this](const Point &p) {
    return state_[p].letter == kEmptyCell;
  });
  absl::flat_hash_set<std::string> combos = top_letters.CombinationsOfSize(k);

  // Get the permutations of each combination.
  absl::flat_hash_set<std::string> options;
  for (std::string combo : combos) {
    do options.insert(combo);
    while (std::next_permutation(combo.begin(), combo.end()));
  }
  return options;
}

/** * * * **
 * Scoring *
 ** * * * **/

int Solver::LineScore(const std::vector<Point> &line) const {
  const std::string word = GetWord(line);
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

void Solver::UpdateBestState() {
  if (int score = Score(); score > best_score_) {
    best_score_ = score;
    best_state_ = state();
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

/** * * **
 * Words *
 ** * * **/

std::string Solver::GetWord(const std::vector<Point> &line) const {
  const int threshold = (line == bonus_line_) ? 4 : 3;
  const std::string word = LongestAlphaSubstring(state().LineString(line));
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
        state().grid()[row],
        [](const Cell &cell) { return cell.letter != kEmptyCell; });
    if (letters > most_letters_placed) {
      most_letters_placed = letters;
      row_to_focus = row;
    }
  }
  return row_to_focus;
}

}  // namespace puzzmo::bongo