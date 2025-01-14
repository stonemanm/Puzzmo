#include "dictionary_utils.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "absl/log/log.h"

namespace puzzmo {

std::vector<std::string>
ReadDictionaryFileToVector(const ReadFileOptions options) {
  std::vector<std::string> words;
  // LOG(ERROR) << options.letter_counts;

  std::ifstream dictfile("data/dictionary.txt");
  if (!dictfile.is_open()) {
    LOG(ERROR) << "Error: Could not open dictionary.txt";
    return words;
  }

  std::string line;
  while (std::getline(dictfile, line)) {
    int l = line.length();
    if (l < options.min_letters || l > options.max_letters)
      continue;
    if (options.filter_by_letters) {
      std::vector<int> letters = options.letter_count.count;
      for (const char c : line) {
        if (--letters[c - 'a'] < 0)
          continue;
      }
    }
    words.push_back(line);
  }
  dictfile.close();
  return words;
};

const absl::flat_hash_map<LetterCount, absl::flat_hash_set<std::string>>
CreateAnagramDictionary(const std::vector<std::string> words) {
  absl::flat_hash_map<LetterCount, absl::flat_hash_set<std::string>> dict;
  for (const auto &word : words) {
    LetterCount lc(word);
    dict[lc].insert(word);
  }
  return dict;
}

const std::shared_ptr<TrieNode>
CreateDictionaryTrie(const std::vector<std::string> words) {
  std::shared_ptr<TrieNode> dict = std::make_shared<TrieNode>();
  for (const auto &word : words) {
    if (word.length() < 3)
      continue;

    std::shared_ptr<TrieNode> node = dict;
    for (const char c : word) {
      const int i = c - 'a';
      if (node->children[i] == nullptr) {
        node->children[i] = std::make_shared<TrieNode>();
      }
      node = node->children[i];
    }

    node->word = &word;
  }
  return dict;
}

} // namespace puzzmo