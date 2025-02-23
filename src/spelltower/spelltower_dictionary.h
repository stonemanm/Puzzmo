#ifndef spelltower_dictionary_h
#define spelltower_dictionary_h

#include <string>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "src/shared/letter_count.h"

namespace puzzmo {

class SpelltowerDictionary {
 public:
  using SearchableWords = absl::flat_hash_map<
      int, absl::flat_hash_map<LetterCount, absl::flat_hash_set<std::string>>>;

  struct SearchOptions {
    int min_length = 1;
    int max_length = INT_MAX;
    LetterCount min_letters;
    LetterCount max_letters;
    std::string matching_regex;
  };

  SpelltowerDictionary() {};
  absl::Status Init();

  absl::flat_hash_set<std::string> GetMatchingWords(
      const SearchOptions& options) const;

  bool contains(absl::string_view word) const;

 private:
  absl::StatusOr<SearchableWords> TryReadingInAndSortingWords() const;

  absl::StatusOr<absl::flat_hash_set<std::string>> TryReadingInWords() const;

  absl::flat_hash_set<std::string> common_words_;
  absl::flat_hash_set<std::string> valid_words_;
  SearchableWords searchable_words_;
};

}  // namespace puzzmo

#endif  // !spelltower_dictionary_h
