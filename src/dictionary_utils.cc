#include "dictionary_utils.h"

#include <fstream>
#include <string>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/strings/str_cat.h"
#include "re2/re2.h"

ABSL_FLAG(std::string, common_bongo_words_path, "data/words_bongo_common.txt",
          "Input file containing all \"common\" words in Bongo. (Common words "
          "are worth 1.3x when scored.)");
ABSL_FLAG(std::string, bongo_words_path, "data/words_bongo.txt",
          "Input file containing all legal words for Bongo.");
ABSL_FLAG(std::string, puzzmo_words_path, "data/words_puzzmo.txt",
          "Input file containing all legal words.");

namespace puzzmo {

absl::StatusOr<std::vector<std::string>> ReadDictionaryFileToVector(
    const ReadFileOptions options) {
  std::string path;
  switch (options.word_source) {
    case WordSet::kPuzzmoWords:
      path = absl::GetFlag(FLAGS_puzzmo_words_path);
      break;
    case WordSet::kBongoWords:
      path = absl::GetFlag(FLAGS_bongo_words_path);
      break;
    case WordSet::kCommonBongoWords:
      path = absl::GetFlag(FLAGS_common_bongo_words_path);
      break;
  }
  std::ifstream dictfile(path);
  if (!dictfile.is_open()) {
    return absl::InvalidArgumentError(
        absl::StrCat("Error: Could not open ", path));
  }

  std::vector<std::string> words;
  std::string line;
  while (std::getline(dictfile, line)) {
    int l = line.length();
    if (l < options.min_letters || l > options.max_letters) continue;
    if (options.letter_count.empty()) {
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
    if (key.contains(lc)) {
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
    if (lc.contains(key)) {
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

const std::shared_ptr<TrieNode> CreateDictionaryTrie(
    const std::vector<std::string> words) {
  std::shared_ptr<TrieNode> dict = std::make_shared<TrieNode>();
  for (const auto &word : words) {
    if (word.length() < 3) continue;

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

}  // namespace puzzmo