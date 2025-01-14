#include "bongo_solver.h"

#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"

namespace puzzmo {

absl::flat_hash_set<absl::flat_hash_set<std::string>>
BongoSolver::FindWordSets() {
  // LOG(ERROR) << absl::StrCat("[", absl::StrJoin(tiles_, ", "), "]");
  absl::flat_hash_set<std::string> current_set;
  absl::flat_hash_set<absl::flat_hash_set<std::string>> word_sets;

  FindWordSetsHelper(tiles_, 0, current_set, word_sets);
  // int x = 10;
  // LOG(ERROR) << "yahaha!";
  // for (const auto &ws : word_sets) {
  //   if (--x < 0)
  //     break;
  //   LOG(ERROR) << "[" << absl::StrJoin(ws, ", ") << "]";
  // }
  return word_sets;
}

void BongoSolver::FindWordSetsHelper(
    LetterCount remaining_letters, int start_at_dict_index,
    absl::flat_hash_set<std::string> &current_set,
    absl::flat_hash_set<absl::flat_hash_set<std::string>> &word_sets) {
  if (current_set.size() == 5) {
    word_sets.insert(current_set);
    return;
  }
  for (int i = start_at_dict_index; i < dict_.size(); ++i) {
    const std::string word = dict_[i];

    bool possible = true;
    LetterCount letters(remaining_letters);
    for (char c : word) {
      if (--letters.count[c - 'a'] < 0) {
        possible = false;
        break;
      }
    }
    if (!possible)
      continue;
    current_set.insert(word);
    FindWordSetsHelper(letters, i, current_set, word_sets);
    current_set.erase(word);
  }
}

} // namespace puzzmo