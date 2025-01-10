#ifndef dictionary_utils_h
#define dictionary_utils_h

#include <climits>
#include <string>
#include <vector>

namespace puzzmo {

// Configuration options for ReadDictionaryFileToVector
struct ReadFileOptions {
  int min_letters = 1;
  int max_letters = INT_MAX;
  bool filter_by_letters = false;
  std::vector<int> letter_counts;
};

// A basic implementation of a node in a Trie.
struct TrieNode {
  std::vector<std::shared_ptr<TrieNode>> children;
  const std::string *word = nullptr;
  TrieNode() : children(26) {}
};

// Returns a vector containing all strings from the file that meet the criteria
std::vector<std::string>
ReadDictionaryFileToVector(const ReadFileOptions options);

// Add one or more words to the trie.
const std::shared_ptr<TrieNode>
CreateDictionaryTrie(const std::vector<std::string> words);

} // namespace puzzmo

#endif