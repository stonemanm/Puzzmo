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

// Solvers

absl::StatusOr<Gamestate> Solver::Solve() {
  return SolveWithTechniquesInOrder(
      {Technique::kFillBonusWord, Technique::kFillMultiplierTiles});
}

absl::StatusOr<Gamestate> Solver::SolveWithTechniquesInOrder(
    absl::Span<const Technique> techniques) {
  if (absl::Status s = RecursiveHelper(techniques, 0); !s.ok()) {
    LOG(ERROR) << s;
    return s;
  }
  // Loop again if nothing found.
  if (best_score_ == 0) {
    ++tiles_for_bonus_words_;
    ++tiles_for_multiplier_tiles_;
    LOG(INFO) << "No solutions found. Trying again with a broader search.";
    return SolveWithTechniquesInOrder(techniques);
  }
  return best_state_;
}

// Mutators

absl::Status Solver::RecursiveHelper(absl::Span<const Technique> techniques,
                                     int i) {
  Gamestate state = CurrentState();
  // Check for failure (error, really)
  if (state.letters().size() < 25)
    return absl::InternalError("Somehow we're losing letters.");

  // Check for success.
  if (IsComplete()) {
    UpdateBestState();
    return absl::OkStatus();
  }

  // When we run out of techniques, finish by finding words recursively.
  Technique t = (i < techniques.size()) ? techniques[i]
                                        : Technique::kFillMostRestrictedRow;

  // Initialize variables used by some techniques.
  std::vector<Point> line;

  // Get the options for the technique.
  absl::flat_hash_set<std::string> options;
  switch (t) {
    case Technique::kFillMostRestrictedRow:
      line = state.line(MostRestrictedWordlessRow());
      options = OptionsForLine(line);
      break;
    case Technique::kFillBonusWord:
      options = OptionsForBonusWord();
      break;
    case Technique::kFillMultiplierTiles:
      options = OptionsForMultiplierTiles();
      break;
  }

  // Try each option.
  int loop = 0;
  for (const absl::string_view option : options) {
    // Place it with the appropriate method.
    absl::Status s;
    switch (t) {
      case Technique::kFillMostRestrictedRow:
        LOG(INFO) << absl::StrFormat(kVerboseLoop, std::string(i + 1, ' '),
                                     ++loop, options.size(), "word", option);
        s = FillLine(line, option);
        break;
      case Technique::kFillBonusWord:
        LOG(INFO) << absl::StrFormat(kVerboseLoop, std::string(i + 1, ' '),
                                     ++loop, options.size(), "bonus word",
                                     option);
        s = FillLine(bonus_line_, option);
        break;
      case Technique::kFillMultiplierTiles:
        LOG(INFO) << absl::StrFormat(kVerboseLoop, std::string(i + 1, ' '),
                                     ++loop, options.size(), "letters", option);
        s = FillMultiplierCells(option);
        break;
    }
    if (!s.ok()) {
      LOG(ERROR) << s;
      return s;
    }
    // Recurse.
    if (absl::Status s = RecursiveHelper(techniques, i + 1); !s.ok()) {
      LOG(ERROR) << s;
      return s;
    }
    // Undo the placement.
    steps_.pop_back();
  }
  return absl::OkStatus();
}

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

absl::Status Solver::FillMultiplierCells(const absl::string_view letters) {
  Gamestate state = CurrentState();
  int i = 0;
  for (const Point &p : multiplier_points_) {
    if (std::isalpha(state[p].letter)) continue;
    if (i >= letters.size()) break;
    if (absl::Status s = state.FillCell(p, letters[i]); !s.ok()) {
      LOG(ERROR) << s;
      return s;
    }
    ++i;
  }
  steps_.push_back(state);
  return absl::OkStatus();
}

absl::flat_hash_set<std::string> Solver::OptionsForBonusWord() const {
  const Gamestate state = CurrentState();
  const LetterCount line_contents(state.LineString(bonus_line_));
  Dict::SearchParameters params = {
      .min_length = 4,
      .max_length = 4,
      .max_letters = state.unplaced_letters() + line_contents,
      .matching_regex = state.LineRegex(bonus_line_)};

  // We narrow the possible bonus words by requiring they use a certain number
  // of the most valuable tiles.
  const LetterCount top_letters(
      state.NMostValuableLetters(tiles_for_bonus_words_));
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
  const Gamestate state = CurrentState();
  const LetterCount line_contents(state.LineString(line));
  const int n = line.size();
  return dict_.WordsMatchingParameters(
      {.min_length = n,  // TODO: 3
       .max_length = n,
       .min_letters = line_contents,
       .max_letters = line_contents + state.unplaced_letters(),
       .matching_regex = state.LineRegex(line)});
}

absl::flat_hash_set<std::string> Solver::OptionsForMultiplierTiles() const {
  const Gamestate state = CurrentState();

  // We only consider a certain number of high-value letters to place on the
  // multiplier tiles.
  const LetterCount top_letters(
      state.NMostValuableLetters(tiles_for_multiplier_tiles_));
  LOG(INFO) << "top_letters == " << top_letters;
  const int k = absl::c_count_if(multiplier_points_, [state](const Point &p) {
    return state[p].letter == kEmptyCell;
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