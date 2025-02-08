#ifndef dictionary_utils_h
#define dictionary_utils_h

#include <climits>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/status/statusor.h"

#include "letter_count.h"

namespace puzzmo {

// Configuration options for ReadDictionaryFileToVector
struct ReadFileOptions {
  int min_letters = 1;
  int max_letters = INT_MAX;
  LetterCount letter_count;
};

// A basic implementation of a node in a Trie.
struct TrieNode {
  std::vector<std::shared_ptr<TrieNode>> children;
  const std::string *word = nullptr;
  TrieNode() : children(26) {}
};

// Returns a vector containing all strings from the file that meet the criteria
absl::StatusOr<std::vector<std::string>>
ReadDictionaryFileToVector(const ReadFileOptions options);

// Preprocess words to get their letter counts
const absl::flat_hash_map<LetterCount, absl::flat_hash_set<std::string>>
CreateAnagramDictionary(const std::vector<std::string> words);

const absl::flat_hash_set<std::string> FindMatchesInAnagramDictionary(
    const absl::flat_hash_map<LetterCount, absl::flat_hash_set<std::string>>
        dict,
    const LetterCount &lc);

const absl::flat_hash_set<std::string> FindMatchesInAnagramDictionary(
    const absl::flat_hash_map<LetterCount, absl::flat_hash_set<std::string>>
        dict,
    const LetterCount &lc, absl::string_view rgx);

// Add one or more words to the trie.
const std::shared_ptr<TrieNode>
CreateDictionaryTrie(const std::vector<std::string> words);

} // namespace puzzmo

#endif