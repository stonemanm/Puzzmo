#include "bongo_solver.h"

#include <algorithm>

#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"

#include "dictionary_utils.h"

namespace puzzmo {

namespace {

// std::string LetterSetString(absl::flat_hash_set<LetterCount> set) {
//   return absl::StrCat("[", absl::StrJoin(set, ", "), "]");
// }

} // namespace

BongoSolver::BongoSolver(const BongoGameState &bgs,
                         const std::vector<std::string> &words)
    : starting_state_(bgs) {

  for (const auto &word : words) {
    LetterCount lc(word);
    if (word.length() == 5) {
      letters_to_words_[lc].insert(word);
    } else {
      letters_to_bonus_words_[lc].insert(word);
    }
  }
  // keys_.reserve(dict_.size());
  // for (const auto &[k, v] : dict_) {
  //   keys_.push_back(k);
  // }
  // std::sort(keys_.begin(), keys_.end());
};

absl::flat_hash_set<absl::flat_hash_set<std::string>>
BongoSolver::FindWordSets() {
  LOG(ERROR) << absl::StrCat("Words in dict: ", letters_to_words_.size());
  absl::flat_hash_set<LetterCount> current_set;
  absl::flat_hash_set<absl::flat_hash_set<LetterCount>> letter_sets;
  FindLetterSetsHelper(starting_state_.RemainingTiles(), 0, current_set,
                       letter_sets);

  absl::flat_hash_set<absl::flat_hash_set<std::string>> word_sets;
  for (const auto &letter_set : letter_sets) {
    absl::flat_hash_set<std::string> word_set;
    for (const auto &letters : letter_set) {
      word_set.insert(letters.CharsInOrder());
    }
    word_sets.insert(word_set);
  }
  return word_sets;
}

void BongoSolver::FindLetterSetsHelper(
    LetterCount remaining_letters, int starting_index,
    absl::flat_hash_set<LetterCount> &current_set,
    absl::flat_hash_set<absl::flat_hash_set<LetterCount>> &letter_sets) {
  if (current_set.size() == 5) {
    LOG(ERROR)
        << "I don't think this gets used anymore. If you see this, I'm wrong.";
    letter_sets.insert(current_set);
    return;
  }

  // Check every entry in the dictionary to see if letter_cost is possible. If
  // so, pay it and then backtrack after.
  for (int i = starting_index; i < letters_to_words_.size(); ++i) {
    if (current_set.empty() && i % 433 == 0) {
      LOG(ERROR) << i / 433
                 << "/15 of the way through possible first words. It'll get "
                    "faster as it goes.";
      LOG(ERROR) << "We've found " << letter_sets.size() << " sets so far.";
    }

    LetterCount letters = remaining_letters - keys_[i];
    if (!letters.Valid())
      continue;

    // If this would be our fourth word, then letters would be our fifth word.
    // If it's a word, we want to include it too. If not, then it's faster to
    // move on now.
    if (current_set.size() == 3) {
      if (letters_to_words_.contains(letters)) {
        current_set.insert(keys_[i]);
        current_set.insert(letters);
        letter_sets.insert(current_set);
        current_set.erase(letters);
        current_set.erase(keys_[i]);
      }
      return;
    }

    current_set.insert(keys_[i]);
    FindLetterSetsHelper(letters, starting_index, current_set, letter_sets);
    current_set.erase(keys_[i]);
  }
}

} // namespace puzzmo