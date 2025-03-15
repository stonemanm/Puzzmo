#include "solver.h"

#include <algorithm>

#include "absl/container/btree_set.h"
#include "absl/log/log.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"

namespace puzzmo::spelltower {
namespace {

constexpr absl::string_view kVerboseBestGoalWordLoop =
    "Searching %d words of length %d for paths that use %d or more stars.";
constexpr absl::string_view kVerboseFoundPathForWord =
    "Found playable %d* path for \"%s\": %v";
constexpr absl::string_view kVerboseLongestWord =
    "[%03d/%03d] Searching for a path for word \"%s\".";
constexpr absl::string_view kVerboseThreeStarCouldBeBetter =
    "A 3* word of length %d or higher would have a higher multiplier. "
    "Continuing the search in case one can be found.";

constexpr absl::string_view kGoalPathNotPossible =
    "No longer possible--undoing the last word.";
constexpr absl::string_view kNotEnoughStars =
    "Not enough stars remain in the grid for this to succeed.";
constexpr absl::string_view kPathEmptyError =
    "Path is empty and cannot be played.";
constexpr absl::string_view kPathNotContinuousError =
    "Path is noncontinuous and cannot be played: %v.";
constexpr absl::string_view kPathNotOnGridError =
    "Not all tiles in the path are on the grid; therefore, it cannot be "
    "played: %v.";
constexpr absl::string_view kStarLettersNotInWord =
    "Word \"%s\" does not use enough of the star letters (%s).";
constexpr absl::string_view kWordNotInGridError =
    "No possible path for \"%s\" found in grid.";
constexpr absl::string_view kWordNotInTrieError =
    "Word \"%s\" is not contained in the trie.";

int PathHeight(const Path& path) {
  int height = 0;
  for (const std::shared_ptr<Tile>& tile : path.tiles()) height += tile->row();
  return height;
}

// We want to keep the greater of the two paths.
auto path_comparator = [](const Path& lhs, const Path& rhs) {
  if (lhs.MultiplierWhenScored() != rhs.MultiplierWhenScored())
    return lhs.MultiplierWhenScored() < rhs.MultiplierWhenScored();
  if (lhs.size() != rhs.size()) return lhs.size() < rhs.size();
  if (lhs.Delta() != rhs.Delta()) return lhs.Delta() > rhs.Delta();
  if (PathHeight(lhs) != PathHeight(rhs))
    return PathHeight(lhs) < PathHeight(rhs);
  for (int i = 0; i < lhs.size(); ++i)
    if (lhs[i]->coords() != rhs[i]->coords())
      return (lhs[i]->col() != rhs[i]->col()) ? lhs[i]->col() < rhs[i]->col()
                                              : lhs[i]->row() > rhs[i]->row();
  return false;
};

}  // namespace

// Constructors

absl::StatusOr<Solver> Solver::CreateSolverWithSerializedDict(
    const Grid& grid) {
  absl::StatusOr<Dict> dict = Dict::LoadDictFromSerializedTrie();
  if (!dict.ok()) return dict.status();
  return Solver(*dict, grid);
}

absl::StatusOr<Solver> Solver::CreateSolverWithSerializedDict(
    const std::vector<std::string>& grid) {
  absl::StatusOr<Dict> dict = Dict::LoadDictFromSerializedTrie();
  if (!dict.ok()) return dict.status();
  return Solver(*dict, Grid(grid));
}

// Mutators

absl::Status Solver::reset() {
  word_cache_.clear();
  solution_.clear();
  snapshots_.clear();
  word_score_sum_ = 0;
  return grid_.reset();
}

absl::Status Solver::PlayWord(const Path& word) {
  if (word.empty()) return absl::InvalidArgumentError(kPathEmptyError);
  if (!word.IsOnGrid())
    return absl::InvalidArgumentError(
        absl::StrFormat(kPathNotOnGridError, word));
  if (!word.IsContinuous())
    return absl::InvalidArgumentError(
        absl::StrFormat(kPathNotContinuousError, word));
  if (!dict_.contains(word.word()))
    return absl::InvalidArgumentError(
        absl::StrFormat(kWordNotInTrieError, word.word()));

  int word_score = grid_.ScorePath(word);
  snapshots_.push_back(grid_.VisualizePath(word));
  if (absl::Status s = grid_.ClearPath(word); !s.ok()) {
    snapshots_.pop_back();
    return s;
  }
  solution_.push_back(word);
  word_cache_.clear();
  word_score_sum_ += word_score;
  return absl::OkStatus();
}

absl::Status Solver::UndoLastPlay() {
  if (solution_.empty())
    return absl::FailedPreconditionError("No words have been played!");

  if (absl::Status s = grid_.RevertLastClear(); !s.ok()) return s;
  word_score_sum_ -= grid_.ScorePath(solution_.back());
  word_cache_.clear();
  solution_.pop_back();
  snapshots_.pop_back();
  return absl::OkStatus();
}

// Solutions

absl::Status Solver::SolveGreedily() {
  FillWordCache();
  while (!word_cache_.empty()) {
    auto& [score, highest_scoring_words] = *word_cache_.begin();
    Path word = *highest_scoring_words.begin();
    if (absl::Status s = PlayWord(word); !s.ok()) return s;
    FillWordCache();
  }
  return absl::OkStatus();
}

absl::Status Solver::SolveWithOneLongWord() {
  absl::StatusOr<std::vector<Path>> partial_solution = BestPossibleGoalWord();
  if (!partial_solution.ok()) return partial_solution.status();

  for (const Path& path : *partial_solution)
    if (absl::Status s = PlayWord(path); !s.ok()) return s;

  return SolveGreedily();
}

// Helpers

absl::StatusOr<std::vector<Path>> Solver::BestPossibleGoalWord() {
  if (grid_.star_tiles().size() < 3)
    return absl::InvalidArgumentError(kNotEnoughStars);

  // Get the parameters.
  std::vector<LetterCount> column_lcs = grid_.column_letter_counts();
  LetterCount letters_in_grid =
      std::accumulate(column_lcs.begin(), column_lcs.end(), LetterCount());
  std::string two_star_regex = grid_.NStarRegex(2);
  std::string three_star_regex = grid_.NStarRegex(3);

  int min_word_len = 3;
  std::vector<Path> partial_solution;
  // We start by casting a wide (2*) net until we get a path.
  bool include_two_star_words = true;
  for (int len = 28; len >= min_word_len; --len) {
    // Get words of the appropriate length.
    auto words_to_try = dict_.WordsMatchingParameters(
        {.min_length = len,
         .max_length = len,
         .letter_superset = letters_in_grid,
         .matching_regex =
             include_two_star_words ? two_star_regex : three_star_regex});
    LOG(INFO) << absl::StrFormat(kVerboseBestGoalWordLoop, words_to_try.size(),
                                 len, include_two_star_words ? 2 : 3);

    int ct_for_logging = 0;
    for (const std::string& word : words_to_try) {
      LOG(INFO) << absl::StrFormat(kVerboseLongestWord, ++ct_for_logging,
                                   words_to_try.size(), word);

      // PHASE 1: 2* or 3*
      if (include_two_star_words) {
        absl::StatusOr<std::vector<Path>> s =
            BestPossibleTwoStarPathForWord(word);
        if (!s.ok()) continue;

        partial_solution = *std::move(s);
        int stars_in_goal_word = partial_solution.back().star_count();
        LOG(INFO) << absl::StrFormat(kVerboseFoundPathForWord,
                                     stars_in_goal_word, word, len);
        if (stars_in_goal_word == 3) return partial_solution;
        min_word_len = len * 3 / 4 + (len * 3 % 4 != 0);
        LOG(INFO) << absl::StrFormat(kVerboseThreeStarCouldBeBetter,
                                     min_word_len);
      }

      // PHASE 2: only 3*
      else {
        absl::StatusOr<std::vector<Path>> s =
            BestPossibleThreeStarPathForWord(word);
        if (!s.ok()) continue;
        LOG(INFO) << absl::StrFormat(kVerboseFoundPathForWord, 3, word, len);
        return *std::move(s);
      }
    }
  }
  if (partial_solution.empty()) return absl::NotFoundError("No words found.");
  return partial_solution;
}

// BestPossiblePathForWord()
absl::StatusOr<Path> Solver::BestPossiblePathForWord(
    absl::string_view word) const {
  if (!dict_.contains(word))
    return absl::InvalidArgumentError(
        absl::StrFormat(kWordNotInTrieError, word));
  Path path;
  Path best_path;
  BestPathDFS(word, 0, path, best_path);
  if (best_path.empty())
    return absl::NotFoundError(absl::StrFormat(kWordNotInGridError, word));
  return best_path;
}

void Solver::BestPathDFS(absl::string_view word, int i, Path& path,
                         Path& best_path) const {
  // Check for success.
  if (i == word.length()) {
    best_path = std::max(path, best_path, path_comparator);
    return;
  }

  // Try all the options.
  absl::flat_hash_set<std::shared_ptr<Tile>> options =
      grid_.letter_map()[word[i]];
  for (const std::shared_ptr<Tile>& next : options) {
    if (absl::Status s = path.push_back(next); !s.ok()) continue;
    BestPathDFS(word, i + 1, path, best_path);
    path.pop_back();
  }
}

// BestPossibleTwoStarPathForWord()
absl::StatusOr<std::vector<Path>> Solver::BestPossibleTwoStarPathForWord(
    absl::string_view word) {
  if (grid_.star_tiles().size() < 2)
    return absl::InvalidArgumentError(kNotEnoughStars);
  if (!dict_.contains(word))
    return absl::InvalidArgumentError(
        absl::StrFormat(kWordNotInTrieError, word));

  // Make sure word has enough of the star letters.
  LetterCount lc(word);
  LetterCount star_letters(
      absl::StrJoin(grid_.star_tiles(), "",
                    [](std::string* out, const std::shared_ptr<Tile>& tile) {
                      out->push_back(tile->letter());
                    }));
  bool has_letters = false;
  for (absl::string_view subset : star_letters.CombinationsOfSize(2)) {
    if (lc.contains(subset)) {
      has_letters = true;
      break;
    }
  }
  if (!has_letters)
    return absl::InvalidArgumentError(absl::StrFormat(
        kStarLettersNotInWord, word, star_letters.CharsInOrder()));

  Path path;
  return TwoStarDFS(word, 0, star_letters, path);
}

absl::StatusOr<std::vector<Path>> Solver::TwoStarDFS(
    absl::string_view word, int i, LetterCount& unused_star_letters,
    Path& path) {
  // Check for success.
  if (i == word.length()) {
    if (path.star_count() < 2)
      return absl::NotFoundError(absl::StrFormat(kWordNotInGridError, word));
    LOG(INFO) << "Trying to find a way to remove words to enable it.";
    return StepsToPlayGoalWordDFS(path);
  }

  // Check for failure. If two of the unused star letters cannot be found in the
  // rest of the word, no need to go further down this branch.
  if (unused_star_letters.size() > 1) {
    int missing = 0;
    for (char c : unused_star_letters.CharsInOrder()) {
      if (!LetterCount(word.substr(i)).contains(c)) ++missing;
      if (missing >= 2)
        return absl::NotFoundError(absl::StrFormat(kWordNotInGridError, word));
    }
  }

  // For every option:
  absl::flat_hash_set<std::shared_ptr<Tile>> options =
      grid_.letter_map()[word[i]];
  for (const std::shared_ptr<Tile>& next : options) {
    // Try the option.
    if (absl::Status s = path.push_back(next); !s.ok()) continue;
    if (next->is_star()) (void)unused_star_letters.RemoveLetter(next->letter());

    // Recurse, returning if we have a partial solution.
    if (absl::StatusOr<std::vector<Path>> s =
            TwoStarDFS(word, i + 1, unused_star_letters, path);
        s.ok())
      return s;

    // Backtrack.
    if (next->is_star()) (void)unused_star_letters.AddLetter(next->letter());
    path.pop_back();
  }
  return absl::NotFoundError(absl::StrFormat(kWordNotInGridError, word));
}

// BestPossibleThreeStarPathForWord()
absl::StatusOr<std::vector<Path>> Solver::BestPossibleThreeStarPathForWord(
    absl::string_view word) {
  if (grid_.star_tiles().size() < 3)
    return absl::InvalidArgumentError(kNotEnoughStars);
  if (!dict_.contains(word))
    return absl::InvalidArgumentError(
        absl::StrFormat(kWordNotInTrieError, word));

  // Make sure word has enough of the star letters.
  LetterCount star_letters(
      absl::StrJoin(grid_.star_tiles(), "",
                    [](std::string* out, const std::shared_ptr<Tile>& tile) {
                      return tile->letter();
                    }));
  if (!LetterCount(word).contains(star_letters))
    return absl::InvalidArgumentError(absl::StrFormat(
        kStarLettersNotInWord, word, star_letters.CharsInOrder()));

  Path path;
  return ThreeStarDFS(word, 0, star_letters, path);
}

absl::StatusOr<std::vector<Path>> Solver::ThreeStarDFS(
    absl::string_view word, int i, LetterCount& unused_star_letters,
    Path& path) {
  // Check for success.
  if (i == word.length()) {
    if (path.star_count() < 3)
      return absl::NotFoundError(absl::StrFormat(kWordNotInGridError, word));
    return StepsToPlayGoalWordDFS(path);
  }

  // Check for failure. If any of the unused star letters cannot be found in
  // the rest of the word, no need to go further down this branch.
  if (!LetterCount(word.substr(i)).contains(unused_star_letters))
    return absl::NotFoundError(absl::StrFormat(kWordNotInGridError, word));

  // For every option:
  absl::flat_hash_set<std::shared_ptr<Tile>> options =
      grid_.letter_map()[word[i]];
  for (const std::shared_ptr<Tile>& next : options) {
    // Try the option.
    if (absl::Status s = path.push_back(next); !s.ok()) continue;
    if (next->is_star()) (void)unused_star_letters.RemoveLetter(next->letter());

    // Recurse, returning if we have a partial solution.
    if (absl::StatusOr<std::vector<Path>> s =
            ThreeStarDFS(word, i + 1, unused_star_letters, path);
        s.ok())
      return s;

    // Backtrack.
    if (next->is_star()) (void)unused_star_letters.AddLetter(next->letter());
    path.pop_back();
  }
  return absl::NotFoundError(absl::StrFormat(kWordNotInGridError, word));
}

void Solver::FillWordCache(
    absl::btree_map<int, absl::btree_set<Path>, std::greater<int>>& cache) {
  if (!cache.empty()) return;

  Path path;
  for (const std::vector<std::shared_ptr<Tile>>& column : grid_.tiles()) {
    for (const std::shared_ptr<Tile>& tile : column) {
      if (absl::Status s = path.push_back(tile); !s.ok()) continue;
      CacheDFS(dict_.trie().root()->children[tile->letter() - 'a'], path,
               cache);
      path.pop_back();
    }
  }
}

void Solver::CacheDFS(
    const std::shared_ptr<TrieNode>& trie_node, Path& path,
    absl::btree_map<int, absl::btree_set<Path>, std::greater<int>>& cache) {
  // Check for failure.
  if (trie_node == nullptr) return;

  // Check for success.
  if (trie_node->is_word) cache[grid_.ScorePath(path)].insert(path);

  absl::flat_hash_set<std::shared_ptr<Tile>> options =
      grid_.PossibleNextTilesForPath(path);
  for (const std::shared_ptr<Tile>& next : options) {
    if (absl::Status s = path.push_back(next); !s.ok()) continue;
    CacheDFS(trie_node->children[next->letter() - 'a'], path, cache);
    path.pop_back();
  }
}

absl::StatusOr<std::vector<Path>> Solver::StepsToPlayGoalWordDFS(
    const Path& goal_word) {
  // Check for success
  if (goal_word.IsContinuous()) {
    std::vector<Path> partial_solution = solution_;
    partial_solution.push_back(goal_word);
    // Since we don't undo our plays on the way out, we need to call reset now.
    if (absl::Status s = reset(); !s.ok()) return s;
    return partial_solution;
  }

  // Check for failure
  if (!goal_word.IsStillPossible())
    return absl::OutOfRangeError(kGoalPathNotPossible);

  // Get all options by calling CacheDFS. We store them locally rather than
  // using `word_cache_` because backtracking would continually clear it.
  absl::btree_map<int, absl::btree_set<Path>, std::greater<int>> cache;
  FillWordCache(cache);
  for (const auto& [_, paths] : cache) {
    for (const Path& path : paths) {
      std::vector<std::shared_ptr<Tile>> affected_tiles =
          grid_.TilesRemovedBy(path);
      bool interferes_with_goal_path = false;
      for (const std::shared_ptr<Tile>& tile : goal_word.tiles()) {
        if (absl::c_contains(affected_tiles, tile)) {
          interferes_with_goal_path = true;
          break;
        }
      }
      if (interferes_with_goal_path) continue;

      // For each viable option, use it, recurse, then backtrack if
      // unsuccessful.
      if (absl::Status s = PlayWord(path); !s.ok()) continue;
      if (absl::StatusOr<std::vector<Path>> s =
              StepsToPlayGoalWordDFS(goal_word);
          s.ok())
        return s;

      if (absl::Status s = UndoLastPlay(); !s.ok())
        return s;  // Shouldn't happen, but if it does we want to see the error!
    }
  }
  return absl::NotFoundError(
      absl::StrFormat(kWordNotInGridError, goal_word.word()));
}

absl::Status Solver::PlayGoalWord(const Path& goal_word) {
  absl::StatusOr<std::vector<Path>> steps = StepsToPlayGoalWordDFS(goal_word);
  if (!steps.ok()) return steps.status();

  for (const Path& path : *steps)
    if (absl::Status s = PlayWord(path); !s.ok()) return s;

  return absl::OkStatus();
}

}  // namespace puzzmo::spelltower