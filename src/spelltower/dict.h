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

  explicit Dict(const Trie& trie)
      : trie_(trie), words_(trie.WordsWithPrefix("")) {}

  static absl::StatusOr<Dict> LoadDictFromSerializedTrie();

  bool contains(absl::string_view word) const { return trie_.contains(word); }

  const Trie& trie() const { return trie_; }

  const absl::flat_hash_set<std::string>& words() const { return words_; }

  // absl::Status Init();

  // absl::flat_hash_set<std::string> GetMatchingWords(
  //     const SearchOptions& options) const;

 private:
  Dict(const Trie& trie, const absl::flat_hash_set<std::string>& words)
      : trie_(trie), words_(words) {}

  const Trie trie_;
  const absl::flat_hash_set<std::string> words_;

  // absl::StatusOr<SearchableWords> TryReadingInAndSortingWords() const;
  // absl::StatusOr<absl::flat_hash_set<std::string>> TryReadingInWords() const;
  // absl::flat_hash_set<std::string> common_words_;
  // SearchableWords searchable_words_;
};

}  // namespace puzzmo::spelltower

#endif  // !dict_h
