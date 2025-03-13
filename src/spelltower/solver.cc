#include "solver.h"

#include "absl/container/btree_set.h"
#include "absl/log/log.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"

namespace puzzmo::spelltower {

constexpr absl::string_view kNotEnoughStars =
    "Not enough stars remain in the grid for this to succeed.";
constexpr absl::string_view kPathEmptyError =
    "Path is empty and cannot be played.";
constexpr absl::string_view kPathNotContinuousError =
    "Path is noncontinuous and cannot be played: %v.";
constexpr absl::string_view kStarLettersNotInWord =
    "Word \"%s\" does not use enough of the star letters (%s).";
constexpr absl::string_view kVerboseLongestWord =
    "[%d/%d] Searching for a path for word \"%s\".";
constexpr absl::string_view kWordNotInGridError =
    "No possible path for \"%s\" found in grid.";
constexpr absl::string_view kWordNotInTrieError =
    "Word \"%s\" is not contained in the trie.";

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

void Solver::reset() {
  grid_ = starting_grid_;
  word_cache_.clear();
  solution_.clear();
  snapshots_.clear();
  word_score_sum_ = 0;
}

absl::Status Solver::PlayWord(const Path& word) {
  if (word.empty()) return absl::InvalidArgumentError(kPathEmptyError);
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

// Helpers

absl::StatusOr<Path> Solver::BestPossibleGoalWord() const {
  if (grid_.star_tiles().size() < 3)
    return absl::InvalidArgumentError(kNotEnoughStars);

  // Get the letters.
  std::vector<LetterCount> column_lcs = grid_.column_letter_counts();
  LetterCount letters_in_grid =
      std::accumulate(column_lcs.begin(), column_lcs.end(), LetterCount());

  // Get the regexes.
  std::string two_star_regex = grid_.NStarRegex(2);
  std::string three_star_regex = grid_.NStarRegex(3);

  Path best_two_star_path;
  // We start by casting a wide (2*) net until we get a path.
  for (int len = 28; len >= 3; --len) {
    // If we have `best_two_star_path`, we determine when to stop looking for a
    // 3* one.
    if (4 * len < 3 * best_two_star_path.size()) return best_two_star_path;
    LOG(INFO) << "";
    LOG(INFO) << absl::StrCat(
        "Searching words of length ", len, " for possible ",
        best_two_star_path.empty() ? "2* or " : "", "3* words.");

    auto words_to_try = dict_.WordsMatchingParameters(
        {.min_length = len,
         .max_length = len,
         .letter_superset = letters_in_grid,
         .matching_regex =
             best_two_star_path.empty() ? two_star_regex : three_star_regex});
    LOG(INFO) << absl::StrCat(words_to_try.size(), " candidates found.");

    int ct = 0;
    for (const std::string& word : words_to_try) {
      LOG(INFO) << absl::StrFormat(kVerboseLongestWord, ++ct,
                                   words_to_try.size(), word);
      if (word == "compassionatenesses") continue;
      if (best_two_star_path.empty()) {
        absl::StatusOr<Path> path = BestPossibleTwoStarPathForWord(word);
        if (!path.ok()) continue;

        // If we have all the stars used, this is the best case! Just return.
        if (path->star_count() == 3) {
          LOG(INFO) << "Found 3* path for \"" << word << "\"!\n" << *path;
          return *std::move(path);
        }
        // Otherwise, we save the two-star path and continue searching with
        // a tighter net.
        best_two_star_path = *path;
        LOG(INFO) << "Found 2* path for \"" << word << "\"!\n"
                  << best_two_star_path;
        LOG(INFO) << "A 3* word of length "
                  << best_two_star_path.size() * 3 / 4 +
                         (best_two_star_path.size() * 3 % 4 != 0)
                  << " or higher would have a higher multiplier. Continuing to "
                     "search.";
      } else {
        absl::StatusOr<Path> path = BestPossibleThreeStarPathForWord(word);
        if (!path.ok()) continue;

        LOG(INFO) << "Found 3* path for \"" << word << "\"!\n" << *path;
        return *std::move(path);
      }
    }
  }
  return absl::NotFoundError("No all-star words found.");
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
    int multiplier = path.MultiplierWhenScored();
    int best_multiplier = best_path.MultiplierWhenScored();
    if (multiplier > best_multiplier ||
        (multiplier == best_multiplier && path.Delta() < best_path.Delta())) {
      best_path = path;
    }
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
absl::StatusOr<Path> Solver::BestPossibleTwoStarPathForWord(
    absl::string_view word) const {
  if (grid_.star_tiles().size() < 2)
    return absl::InvalidArgumentError(kNotEnoughStars);
  if (!dict_.contains(word))
    return absl::InvalidArgumentError(
        absl::StrFormat(kWordNotInTrieError, word));

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
  Path best_path;
  TwoStarDFS(word, 0, star_letters, path, best_path);
  if (best_path.empty())
    return absl::NotFoundError(absl::StrFormat(kWordNotInGridError, word));
  return best_path;
}

void Solver::TwoStarDFS(absl::string_view word, int i,
                        LetterCount& unused_star_letters, Path& path,
                        Path& best_path) const {
  // Check for success.
  if (i == word.length()) {
    if (path.star_count() >= 2) {
      if (best_path.MultiplierWhenScored() < path.MultiplierWhenScored() ||
          (best_path.MultiplierWhenScored() == path.MultiplierWhenScored() &&
           path.Delta() < best_path.Delta())) {
        best_path = path;
      }
    }
    return;
  }

  // Check for failure. If two of the unused star letters cannot be found in the
  // rest of the word, no need to go further down this branch.
  if (unused_star_letters.size() > 1) {
    int missing = 0;
    for (char c : unused_star_letters.CharsInOrder()) {
      if (!LetterCount(word.substr(i)).contains(c)) ++missing;
      if (missing == 2) return;
    }
  }

  // Try all the options.
  absl::flat_hash_set<std::shared_ptr<Tile>> options =
      grid_.letter_map()[word[i]];
  for (const std::shared_ptr<Tile>& next : options) {
    if (absl::Status s = path.push_back(next); !s.ok()) continue;
    if (next->is_star()) (void)unused_star_letters.RemoveLetter(next->letter());
    TwoStarDFS(word, i + 1, unused_star_letters, path, best_path);
    if (next->is_star()) (void)unused_star_letters.AddLetter(next->letter());
    path.pop_back();
  }
}

// BestPossibleThreeStarPathForWord()
absl::StatusOr<Path> Solver::BestPossibleThreeStarPathForWord(
    absl::string_view word) const {
  if (grid_.star_tiles().size() < 3)
    return absl::InvalidArgumentError(kNotEnoughStars);
  if (!dict_.contains(word))
    return absl::InvalidArgumentError(
        absl::StrFormat(kWordNotInTrieError, word));

  LetterCount star_letters(
      absl::StrJoin(grid_.star_tiles(), "",
                    [](std::string* out, const std::shared_ptr<Tile>& tile) {
                      return tile->letter();
                    }));
  if (!LetterCount(word).contains(star_letters))
    return absl::InvalidArgumentError(absl::StrFormat(
        kStarLettersNotInWord, word, star_letters.CharsInOrder()));

  Path path;
  Path best_path;
  ThreeStarDFS(word, 0, star_letters, path, best_path);
  if (best_path.empty())
    return absl::NotFoundError(absl::StrFormat(kWordNotInGridError, word));
  return best_path;
}

void Solver::ThreeStarDFS(absl::string_view word, int i,
                          LetterCount& unused_star_letters, Path& path,
                          Path& best_path) const {
  // Check for success.
  if (i == word.length()) {
    if (path.star_count() == grid_.star_tiles().size()) {
      if (best_path.size() < word.length() ||
          path.Delta() < best_path.Delta()) {
        best_path = path;
      }
    }
    return;
  }

  // Check for failure. If any of the unused star letters cannot be found in the
  // rest of the word, no need to go further down this branch.
  if (!LetterCount(word.substr(i)).contains(unused_star_letters)) return;

  // Try all the options.
  absl::flat_hash_set<std::shared_ptr<Tile>> options =
      grid_.letter_map()[word[i]];
  // int ct = 0;
  for (const std::shared_ptr<Tile>& next : options) {
    // LOG(INFO) << "Index " << i << ", option " << ++ct << " of "
    //           << options.size();
    if (absl::Status s = path.push_back(next); !s.ok()) continue;
    if (next->is_star()) (void)unused_star_letters.RemoveLetter(next->letter());
    ThreeStarDFS(word, i + 1, unused_star_letters, path, best_path);
    if (next->is_star()) (void)unused_star_letters.AddLetter(next->letter());
    path.pop_back();
  }
}

void Solver::FillWordCache() {
  if (!word_cache_.empty()) return;

  Path path;
  for (const std::vector<std::shared_ptr<Tile>>& column : grid_.tiles()) {
    for (const std::shared_ptr<Tile>& tile : column) {
      if (absl::Status s = path.push_back(tile); !s.ok()) continue;
      CacheDFS(dict_.trie().root()->children[tile->letter() - 'a'], path);
      path.pop_back();
    }
  }
}

void Solver::CacheDFS(const std::shared_ptr<TrieNode>& trie_node, Path& path) {
  // Check for failure.
  if (trie_node == nullptr) return;

  // Check for success.
  if (trie_node->is_word) word_cache_[grid_.ScorePath(path)].insert(path);

  absl::flat_hash_set<std::shared_ptr<Tile>> options =
      grid_.PossibleNextTilesForPath(path);
  for (const std::shared_ptr<Tile>& next : options) {
    if (absl::Status s = path.push_back(next); !s.ok()) continue;
    CacheDFS(trie_node->children[next->letter() - 'a'], path);
    path.pop_back();
  }
}

}  // namespace puzzmo::spelltower