#ifndef bongo_solver_h
#define bongo_solver_h

#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"

#include "letter_count.h"

namespace puzzmo {

class BongoSolver {
public:
  BongoSolver(
      const LetterCount tiles,
      const absl::flat_hash_map<LetterCount, absl::flat_hash_set<std::string>>
          &dict);

  absl::flat_hash_set<absl::flat_hash_set<std::string>> FindWordSets();

private:
  LetterCount tiles_;
  std::vector<LetterCount> keys_;
  const absl::flat_hash_map<LetterCount, absl::flat_hash_set<std::string>>
      dict_;

  void FindLetterSetsHelper(
      LetterCount remaining_letters, int starting_index,
      absl::flat_hash_set<LetterCount> &current_set,
      absl::flat_hash_set<absl::flat_hash_set<LetterCount>> &letter_sets);
};

} // namespace puzzmo

#endif