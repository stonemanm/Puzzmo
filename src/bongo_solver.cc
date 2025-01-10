#include "bongo_solver.h"

namespace puzzmo {

absl::flat_hash_set<absl::flat_hash_set<std::string>>
BongoSolver::FindWordSets() {
  absl::flat_hash_set<std::string> current_set;
  absl::flat_hash_set<absl::flat_hash_set<std::string>> word_sets;

  FindWordSetsHelper(letters_, current_set, word_sets);
  return word_sets;
}

void BongoSolver::FindWordSetsHelper(
    std::vector<int> remaining_letters,
    absl::flat_hash_set<std::string> &current_set,
    absl::flat_hash_set<absl::flat_hash_set<std::string>> &word_sets) {
  if (current_set.size() == 5) {
    word_sets.insert(current_set);
    return;
  }
  for (const std::string &word : dict_) {
    std::vector<int> letters(remaining_letters);
    for (char c : word) {
      if (--letters[c - 'a'] < 0)
        continue;
    }
    current_set.insert(word);
    FindWordSetsHelper(letters, current_set, word_sets);
    current_set.erase(word);
  }
}

} // namespace puzzmo