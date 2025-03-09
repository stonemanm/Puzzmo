// -----------------------------------------------------------------------------
// File: dict.h
// -----------------------------------------------------------------------------
//
// This header file defines a dict: the overarching structure that handles word
// legality for Spelltower. It stores words in both a trie and a sorted hash set
// to fulfill multiple different use cases efficiently.

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

// spelltower::Dict
//
// A class with two data members: a `Trie`, and a map of word sets. The class
// methods use the more efficient structure.
class Dict {
 public:
  //--------------
  // Constructors

  // A static method that creates a `Dict` by loading from a file that contains
  // the serialization of a trie. Simultaneously constructs the `Trie` and the
  // map of words, which is
  static absl::StatusOr<Dict> LoadDictFromSerializedTrie();

  // The constructor called by `LoadDictFromSerializedTrie()`.
  Dict(const Trie& trie,
       const absl::flat_hash_map<LetterCount, absl::flat_hash_set<std::string>>&
           words)
      : trie_(trie), words_(words) {}

  // A `Dict` can be constructed using just a `Trie`, though it's more efficient
  // to call `LoadDictFromSerializedTrie()` and parse them together.
  explicit Dict(const Trie& trie) : Dict(trie, trie.WordsWithPrefix("")) {}

  //-----------
  // Accessors

  // Dict::trie()
  //
  // Provides direct access to the underlying `Trie`.
  const Trie& trie() const { return trie_; }

  // Dict::words()
  //
  // Provides direct access to the underlying map of words.
  const absl::flat_hash_map<LetterCount, absl::flat_hash_set<std::string>>&
  words() const {
    return words_;
  }

  // Dict::contains()
  //
  // Looks up the word directly in `words_`. Runs in O(1) time.
  bool contains(absl::string_view word) const;

  // Dict::NumWordsWithPrefix()
  //
  // Follows `prefix` to a `TrieNode`, where it returns
  // `TrieNode::words_with_prefix`. If the prefix is not in the trie, returns 0.
  int NumWordsWithPrefix(absl::string_view prefix) const {
    return trie_.NumWordsWithPrefix(prefix);
  }

  // Dict::WordsWithPrefix()
  //
  // Follows `prefix` down the `Trie` to a `TrieNode`, from which DFS is
  // performed to obtain all possible words. If the prefix is not in the trie,
  // returns an empty set. Can be passed an empty string to return all words in
  // the trie.
  absl::flat_hash_set<std::string> WordsWithPrefix(
      absl::string_view prefix) const {
    return trie_.WordsWithPrefix(prefix);
  }

  //--------
  // Search

  // Dict::SearchParameters
  //
  // The parameters that can be used when searching for words.
  struct SearchParameters {
    int min_length = 3;
    int max_length = INT_MAX;
    LetterCount letter_subset;
    LetterCount letter_superset;
    // std::string matching_regex;
  };

  // Dict::WordsMatchingParameters()
  //
  // Searches the word map for all words that meet the provided criteria.
  absl::flat_hash_set<std::string> WordsMatchingParameters(
      const SearchParameters& params) const;

 private:
  // We've had constructors, yes, but what about second constructors?
  Dict(const Trie& trie, const absl::flat_hash_set<std::string>& words);

  //---------
  // Members

  const Trie trie_;
  absl::flat_hash_map<LetterCount, absl::flat_hash_set<std::string>> words_;
};

}  // namespace puzzmo::spelltower

#endif  // !dict_h
