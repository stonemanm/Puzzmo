#include "solver.h"

#include "absl/container/btree_set.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"

namespace puzzmo::spelltower {

constexpr absl::string_view kPathEmptyError =
    "Path is empty and cannot be played.";
constexpr absl::string_view kPathNotContinuousError =
    "Path is noncontinuous and cannot be played: %v.";
constexpr absl::string_view kWordNotInGridError =
    "No possible path for \"%s\" found in grid.";
constexpr absl::string_view kStarLettersNotInWord =
    "Word \"%s\" does not use all of the star letters (%s).";
constexpr absl::string_view kWordNotInTrieError =
    "Word \"%s\" is not contained in the trie.";

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

absl::StatusOr<Path> Solver::BestPossiblePathForWord(
    absl::string_view word) const {
  if (!dict_.contains(word))
    return absl::InvalidArgumentError(
        absl::StrFormat(kWordNotInTrieError, word));
  Path path;
  Path best_path;
  BestPossiblePathForWordDFS(word, 0, path, best_path);
  if (best_path.empty())
    return absl::NotFoundError(absl::StrFormat(kWordNotInGridError, word));
  return best_path;
}

absl::StatusOr<Path> Solver::BestPossibleAllStarPathForWord(
    absl::string_view word) const {
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
  BestPossibleAllStarPathForWordDFS(word, 0, star_letters, path, best_path);
  if (best_path.empty())
    return absl::NotFoundError(absl::StrFormat(kWordNotInGridError, word));
  return best_path;
}

absl::StatusOr<Path> Solver::LongestPossibleAllStarWord() const {
  std::vector<LetterCount> col_lcs = grid_.column_letter_counts();
  LetterCount superset =
      std::accumulate(col_lcs.begin(), col_lcs.end(), LetterCount());
  auto words_to_try = dict_.WordsMatchingParameters(
      {.letter_superset = superset, .matching_regex = grid_.AllStarRegex()});
  for (const std::string& word : words_to_try) {
    if (absl::StatusOr<Path> path = BestPossibleAllStarPathForWord(word);
        path.ok())
      return path;
  }
  return absl::NotFoundError("No all-star words found.");
}

void Solver::FillWordCache() {
  if (!word_cache_.empty()) return;

  Path path;
  for (const auto& column : grid_.tiles()) {
    for (const std::shared_ptr<Tile>& tile : column) {
      if (absl::Status s = path.push_back(tile); !s.ok()) continue;
      FillWordCacheDFS(dict_.trie().root()->children[tile->letter() - 'a'],
                       path);
      path.pop_back();
    }
  }
}

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

void Solver::BestPossiblePathForWordDFS(absl::string_view word, int i,
                                        Path& path, Path& best_path) const {
  // Check for success.
  if (i == word.length()) {
    int score = grid_.ScorePath(path);
    int best_score = grid_.ScorePath(best_path);
    if (score > best_score ||
        (score == best_score && path.Delta() < best_path.Delta())) {
      best_path = path;
    }
    return;
  }

  // Try all the options.
  absl::flat_hash_set<std::shared_ptr<Tile>> options =
      grid_.letter_map()[word[i]];
  for (const std::shared_ptr<Tile>& next : options) {
    if (absl::Status s = path.push_back(next); !s.ok()) continue;
    BestPossiblePathForWordDFS(word, i + 1, path, best_path);
    path.pop_back();
  }
}

void Solver::BestPossibleAllStarPathForWordDFS(absl::string_view word, int i,
                                               LetterCount& unused_star_letters,
                                               Path& path,
                                               Path& best_path) const {
  // Check for success.
  if (i == word.length()) {
    if (path.star_count() == grid_.star_tiles().size()) {
      int score = grid_.ScorePath(path);
      int best_score = grid_.ScorePath(best_path);
      if (score > best_score ||
          (score == best_score && path.Delta() < best_path.Delta())) {
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
  for (const std::shared_ptr<Tile>& next : options) {
    if (absl::Status s = path.push_back(next); !s.ok()) continue;
    if (next->is_star()) (void)unused_star_letters.RemoveLetter(next->letter());
    BestPossibleAllStarPathForWordDFS(word, i + 1, unused_star_letters, path,
                                      best_path);
    if (next->is_star()) (void)unused_star_letters.AddLetter(next->letter());
    path.pop_back();
  }
}

void Solver::FillWordCacheDFS(const std::shared_ptr<TrieNode>& trie_node,
                              Path& path) {
  // Check for failure.
  if (trie_node == nullptr) return;

  // Check for success.
  if (trie_node->is_word) word_cache_[grid_.ScorePath(path)].insert(path);

  absl::flat_hash_set<std::shared_ptr<Tile>> options =
      grid_.PossibleNextTilesForPath(path);
  for (const std::shared_ptr<Tile>& next : options) {
    if (absl::Status s = path.push_back(next); !s.ok()) continue;
    FillWordCacheDFS(trie_node->children[next->letter() - 'a'], path);
    path.pop_back();
  }
}

}  // namespace puzzmo::spelltower