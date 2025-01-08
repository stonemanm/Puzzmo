#ifndef dictionary_utils_h
#define dictionary_utils_h

#include <climits>
#include <string>
#include <vector>

namespace puzzmo {

// A basic implementation of a node in a Trie.
struct TrieNode {
  std::vector<std::shared_ptr<TrieNode>> children;
  const std::string *word = nullptr;
  TrieNode() : children(26) {}
};

// Add one or more words to the trie.
void AddToDictionary(const std::vector<std::string> words,
                     const std::shared_ptr<TrieNode> dict);

// Configuration options for ReadDictionaryFileToVector
struct ReadFileOptions {
  int min_letters = 1;
  int max_letters = INT_MAX;
};

// Returns a vector containing all strings from the file that meet the criteria
std::vector<std::string>
ReadDictionaryFileToVector(const std::string f, const ReadFileOptions options);

} // namespace puzzmo

#endif