// -----------------------------------------------------------------------------
// File: dict.h
// -----------------------------------------------------------------------------
//
// This header file defines a dict: the overarching structure that handles word
// legality for Bongo. It stores an additional hashset of "common" words that
// earn a 1.3x score bonus.

#ifndef PUZZMO_BONGO_DICT_H_
#define PUZZMO_BONGO_DICT_H_

#include <string>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "src/shared/letter_count.h"

namespace puzzmo::bongo {

// bongo::Dict
//
// A `Dict` object stores words in two data members:
// - `words_`, a map from a `LetterCount` to all legal anagrams of those
//   letters.
// - `common_words_`, a set containing all words that Bongo counts as "common".
//   Every word in `common_words_` is also in `words_`.
//
// The dictionary can also be searched via `WordsMatchingParameters()`.
class Dict {
 public:
  //--------------
  // Constructors

  // A static method that creates a `Dict` by loading from the files that
  // contain the words and the common words. The preferred way to get a `Dict`.
  static absl::StatusOr<Dict> LoadFromFiles();

  // Constructing a `Dict` requires two sets of words, one just of the common
  // words.
  Dict(const absl::flat_hash_set<std::string>& words,
       const absl::flat_hash_set<std::string>& common_words);

  // The sets can also be sorted before being passed in.
  Dict(const absl::flat_hash_map<LetterCount, absl::flat_hash_set<std::string>>&
           words,
       const absl::flat_hash_map<LetterCount, absl::flat_hash_set<std::string>>&
           common_words)
      : words_(words), common_words_(common_words) {}

  //-----------
  // Accessors

  // Dict::contains()
  //
  // Looks up the word directly in `words_`. Runs in O(1) time.
  bool contains(absl::string_view word) const;

  // Dict::IsCommonWord()
  //
  // Looks up the word directly in `common_words_`. Runs in O(1) time.
  bool IsCommonWord(absl::string_view word) const;

  //--------
  // Search

  // Dict::SearchParameters
  //
  // The parameters that can be used when searching for words.
  struct SearchParameters {
    bool only_common_words = false;
    int min_length = 1;
    int max_length = INT_MAX;
    LetterCount min_letters;
    LetterCount max_letters;
    std::string matching_regex;
  };

  // Dict::WordsMatchingParameters()
  //
  // Returns a set containing all words matching `params`.
  absl::flat_hash_set<std::string> WordsMatchingParameters(
      const SearchParameters& params) const;

 private:
  absl::flat_hash_map<LetterCount, absl::flat_hash_set<std::string>> words_;
  absl::flat_hash_map<LetterCount, absl::flat_hash_set<std::string>>
      common_words_;
};

}  // namespace puzzmo::bongo

#endif