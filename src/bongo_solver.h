#ifndef bongo_solver_h
#define bongo_solver_h

#include <string>
#include <vector>

#include "absl/container/flat_hash_set.h"

namespace puzzmo {

class BongoSolver {
public:
  BongoSolver(const std::vector<int> letters,
              const std::vector<std::string> &dict)
      : letters_(letters), dict_(dict) {};

  absl::flat_hash_set<absl::flat_hash_set<std::string>> FindWordSets();

private:
  std::vector<int> letters_;
  const std::vector<std::string> dict_;

  void FindWordSetsHelper(
      std::vector<int> remaining_letters, int start_at_dict_index,
      absl::flat_hash_set<std::string> &current_set,
      absl::flat_hash_set<absl::flat_hash_set<std::string>> &word_sets);
};

} // namespace puzzmo

#endif