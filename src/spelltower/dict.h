#ifndef dict_h
#define dict_h

#include <string>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "src/shared/letter_count.h"
#include "trie.h"

namespace puzzmo::spelltower {

class Dict {
 public:
  using SearchableWords = absl::flat_hash_map<
      int, absl::flat_hash_map<LetterCount, absl::flat_hash_set<std::string>>>;

  struct SearchOptions {
    int min_length = 3;
    int max_length = INT_MAX;
    LetterCount min_letters;
    LetterCount max_letters;
    std::string matching_regex;
  };

  static absl::StatusOr<Dict> LoadDictFromSerializedTrie();

  bool contains(absl::string_view word) const { return trie_.contains(word); }

  // absl::Status Init();

  // absl::flat_hash_set<std::string> GetMatchingWords(
  //     const SearchOptions& options) const;

 private:
  Dict(const Trie& trie) : trie_(trie) {}

  const Trie trie_;

  // absl::StatusOr<SearchableWords> TryReadingInAndSortingWords() const;
  // absl::StatusOr<absl::flat_hash_set<std::string>> TryReadingInWords() const;
  // absl::flat_hash_set<std::string> common_words_;
  // absl::flat_hash_set<std::string> valid_words_;
  // SearchableWords searchable_words_;
};

}  // namespace puzzmo::spelltower

#endif  // !dict_h
