#include "dictionary_utils.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "re2/re2.h"

ABSL_FLAG(std::string, path_to_word_file, "data/dictionary.txt",
          "Input file containing all legal words.");

namespace puzzmo {

absl::StatusOr<std::vector<std::string>>
ReadDictionaryFileToVector(const ReadFileOptions options) {
  const std::string path = absl::GetFlag(FLAGS_path_to_word_file);
  std::ifstream dictfile(path);
  if (!dictfile.is_open()) {
    return absl::InvalidArgumentError(
        absl::StrCat("Error: Could not open ", path));
  }

  std::vector<std::string> words;
  std::string line;
  while (std::getline(dictfile, line)) {
    int l = line.length();
    if (l < options.min_letters || l > options.max_letters)
      continue;
    if (options.letter_count.Empty()) {
      words.push_back(line);
      continue;
    }
    LetterCount lc = options.letter_count;
    bool valid = true;
    for (char c : line) {
      auto n = lc.RemoveLetter(c);
      if (!n.ok()) {
        valid = false;
        break;
      }
    }
    if (valid) {
      words.push_back(line);
    }
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

const absl::flat_hash_set<std::string> FindMatchesInAnagramDictionary(
    const absl::flat_hash_map<LetterCount, absl::flat_hash_set<std::string>>
        dict,
    const LetterCount &lc) {
  absl::flat_hash_set<std::string> words;
  for (const auto &[key, values] : dict) {
    if (key.Contains(lc)) {
      absl::flat_hash_set<std::string> matches = dict.at(key);
      words.insert(matches.begin(), matches.end());
    }
  }
  return words;
}

const absl::flat_hash_set<std::string> FindMatchesInAnagramDictionary(
    const absl::flat_hash_map<LetterCount, absl::flat_hash_set<std::string>>
        dict,
    const LetterCount &lc, absl::string_view rgx) {
  absl::flat_hash_set<std::string> words;
  for (const auto &[key, values] : dict) {
    if (lc.Contains(key)) {
      absl::flat_hash_set<std::string> matches = dict.at(key);
      for (const auto &word : matches) {
        if (RE2::FullMatch(word, rgx)) {
          words.insert(word);
        }
      }
    }
  }
  return words;
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