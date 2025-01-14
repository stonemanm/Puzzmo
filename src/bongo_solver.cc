#include "bongo_solver.h"

#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"

namespace puzzmo {

absl::flat_hash_set<absl::flat_hash_set<std::string>>
BongoSolver::FindWordSets() {
  absl::flat_hash_set<LetterCount> current_set;
  absl::flat_hash_set<absl::flat_hash_set<LetterCount>> letter_sets;
  FindLetterSetsHelper(tiles_, current_set, letter_sets);

  absl::flat_hash_set<absl::flat_hash_set<std::string>> word_sets;
  for (const auto &letter_set : letter_sets) {
    absl::flat_hash_set<std::string> word_set;
    for (const auto &letters : letter_set) {
      word_set.insert(letters.toString());
    }
    word_sets.insert(word_set);
  }
  return word_sets;
}

void BongoSolver::FindLetterSetsHelper(
    LetterCount remaining_letters,
    absl::flat_hash_set<LetterCount> &current_set,
    absl::flat_hash_set<absl::flat_hash_set<LetterCount>> &letter_sets) {
  if (current_set.size() == 5) {
    letter_sets.insert(current_set);
    return;
  }

  // Check every entry in the dictionary to see if letter_cost is possible. If
  // so, pay it and then backtrack after.
  for (const auto &[letter_cost, _] : dict_) {
    LetterCount letters = remaining_letters - letter_cost;
    if (!letters.isValid())
      continue;

    current_set.insert(letter_cost);
    FindLetterSetsHelper(letters, current_set, letter_sets);
    current_set.erase(letter_cost);
  }
}

} // namespace puzzmo