#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "absl/log/log.h"
#include "dictionary-utils.h"

namespace puzzmo {

void AddToDictionary(const std::vector<std::string> words,
                     const std::shared_ptr<TrieNode> dict) {
  for (const auto &word : words) {
    if (word.length() < 3)
      continue;

    std::shared_ptr<TrieNode> node = dict;
    for (const char c : word) {
      const int i = c - 'a';
      if (node->children[i] == nullptr)
        node->children[i] = std::make_shared<TrieNode>();

      node = node->children[i];
    }

    node->word = &word;
  }
}

std::vector<std::string>
ReadDictionaryFileToVector(const ReadFileOptions options) {
  std::vector<std::string> words;

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
    words.push_back(line);
  }
  dictfile.close();
  return words;
};

} // namespace puzzmo