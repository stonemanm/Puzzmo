#ifndef bongo_h
#define bongo_h

#include <string>
#include <vector>

#include "absl/container/flat_hash_set.h"

namespace puzzmo {

class Bongo {
public:
  Bongo(const absl::flat_hash_set<std::string> &dict,
        const std::string &letters);

  absl::flat_hash_set<absl::flat_hash_set<std::string>> FindWordSets();

private:
  std::vector<int> letters_;
  const absl::flat_hash_set<std::string> dict_;

  void FindWordSetsHelper(
      std::vector<int> remaining_letters,
      absl::flat_hash_set<std::string> &current_set,
      absl::flat_hash_set<absl::flat_hash_set<std::string>> &word_sets);
};

} // namespace puzzmo

#endif