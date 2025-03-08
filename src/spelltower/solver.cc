#include "solver.h"
namespace puzzmo::spelltower {

absl::StatusOr<Solver> Solver::CreateSolverWithSerializedTrie(
    const Grid& grid) {
  absl::StatusOr<Trie> trie = Trie::LoadFromSerializedTrie();
  if (!trie.ok()) return trie.status();
  return Solver(*trie, grid);
}

absl::StatusOr<Solver> Solver::CreateSolverWithSerializedTrie(
    const std::vector<std::string>& grid) {
  absl::StatusOr<Trie> trie = Trie::LoadFromSerializedTrie();
  if (!trie.ok()) return trie.status();
  return Solver(*trie, Grid(grid));
}

absl::StatusOr<Path> Solver::BestPossiblePathForWord(
    absl::string_view word) const {
  if (!trie_.contains(word))
    return absl::InvalidArgumentError("Word not in trie.");
  Path path;
  Path best_path;
  BestPossiblePathForWordDFS(word, 0, path, best_path);
  if (best_path.empty())
    return absl::NotFoundError(
        absl::StrCat("No possible path for ", word, " found in grid."));
  return best_path;
}

void Solver::FillWordCache() {
  if (!word_cache_.empty()) return;

  Path path;
  for (const auto& column : grid_.tiles()) {
    for (const std::shared_ptr<Tile>& tile : column) {
      if (absl::Status s = path.push_back(tile); !s.ok()) continue;
      FillWordCacheDFS(trie_.root()->children[tile->letter() - 'a'], path);
      path.pop_back();
    }
  }
}

void Solver::reset() {
  grid_ = starting_grid_;
  word_cache_.clear();
  solution_.clear();
  snapshots_.clear();
  words_score_ = 0;
}

absl::Status Solver::PlayWord(const Path& word) {
  if (word.empty())
    return absl::InvalidArgumentError("Path is empty and cannot be played.");
  if (!word.IsContinuous())
    return absl::InvalidArgumentError(
        "Path is not continuous and cannot be played.");
  if (!trie_.contains(word.word()))
    return absl::InvalidArgumentError(
        absl::StrCat("Path contains word ", word.word(),
                     ", which is not contained in the trie."));

  int n = grid_.ScorePath(word);
  snapshots_.push_back(grid_.VisualizePath(word));
  if (absl::Status s = grid_.ClearPath(word); !s.ok()) {
    snapshots_.pop_back();
    return s;
  }
  solution_.push_back(word);
  word_cache_.clear();
  words_score_ += n;
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
  // TODO: could modify for all-star word by including a failure condition.
  // Could even keep a letter count--if we haven't used the star of a letter and
  // we're out of that letter...

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