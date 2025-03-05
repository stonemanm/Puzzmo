#ifndef dict_h
#define dict_h

#include <string>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "src/shared/letter_count.h"

namespace puzzmo::bongo {

struct SearchKey {
  size_t length = 5;
  LetterCount letters;

  // Allows hashing of SearchKey.
  template <typename H>
  friend H AbslHashValue(H h, const SearchKey& key) {
    return H::combine(std::move(h), key.length, key.letters);
  }
};

bool operator==(const SearchKey& lhs, const SearchKey& rhs);
bool operator!=(const SearchKey& lhs, const SearchKey& rhs);

class Dict {
 public:
  using SearchableWords =
      absl::flat_hash_map<SearchKey, absl::flat_hash_set<std::string>>;

  struct SearchOptions {
    int min_length = 1;
    int max_length = INT_MAX;
    LetterCount min_letters;
    LetterCount max_letters;
    std::string matching_regex;
  };

  Dict() {};
  Dict(const absl::flat_hash_set<std::string>& valid_words,
       const absl::flat_hash_set<std::string>& common_words,
       const SearchableWords& searchable_words)
      : valid_words_(valid_words),
        common_words_(common_words),
        searchable_words_(searchable_words) {}
  static absl::StatusOr<Dict> LoadFromFiles();

  bool IsCommonWord(absl::string_view word) const;
  bool IsValidWord(absl::string_view word) const;

  absl::flat_hash_set<std::string> GetMatchingWords(
      const SearchOptions& options) const;

 private:
  static absl::StatusOr<SearchableWords> TryReadingInAndSortingWords();

  static absl::StatusOr<absl::flat_hash_set<std::string>> TryReadingInWords(
      bool common);

  absl::flat_hash_set<std::string> valid_words_;
  absl::flat_hash_set<std::string> common_words_;
  SearchableWords searchable_words_;
};

}  // namespace puzzmo::bongo

#endif
