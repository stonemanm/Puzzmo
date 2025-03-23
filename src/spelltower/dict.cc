#include "dict.h"

#include <fstream>
#include <string>
#include <utility>

#include "absl/flags/flag.h"
#include "absl/log/log.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "re2/re2.h"

ABSL_FLAG(std::string, serialized_dict_path, "data/serialized_trie.txt",
          "Input file containing all legal words for Spelltower, serialized "
          "for ease of loading.");
ABSL_FLAG(std::string, spelltower_words_path, "data/words_puzzmo.txt",
          "Input file containing all legal words for Spelltower.");

namespace puzzmo::spelltower {

absl::StatusOr<Dict> Dict::LoadDictFromSerializedTrie() {
  // Get the serialized trie.
  std::string path = absl::GetFlag(FLAGS_serialized_dict_path);
  std::ifstream file(path);
  if (!file.is_open())
    return absl::InvalidArgumentError(
        absl::StrCat("Error: Could not open ", path));
  std::string serialized_trie_string;
  std::getline(file, serialized_trie_string);
  file.close();

  // Prepare both data structures for the words.
  absl::flat_hash_map<LetterCount, absl::flat_hash_set<std::string>> words;
  Trie trie;

  // Construct both representations of the words at the same time.
  LetterCount lc;
  std::string letter_path;
  std::stack<std::shared_ptr<TrieNode>> node_path;
  std::shared_ptr<TrieNode> node = trie.root();
  for (char c : serialized_trie_string) {
    if (std::isalpha(c)) {
      int idx = c - 'a';
      node->children[idx] = std::make_shared<TrieNode>();
      node_path.push(node);
      node = node->children[idx];
      if (absl::StatusOr<int> s = lc.AddLetter(c); !s.ok()) {
        LOG(ERROR) << s.status();
        return s.status();
      }
      letter_path.push_back(c);
    } else if (std::isdigit(c)) {
      node->words_with_prefix *= 10;
      node->words_with_prefix += c - '0';
    } else if (c == kNodeIsWord) {
      node->is_word = true;
      words[lc].insert(letter_path);
    } else if (c == kEndOfNode) {
      if (node_path.empty()) continue;
      node = node_path.top();
      node_path.pop();
      if (absl::StatusOr<int> s = lc.RemoveLetter(letter_path.back());
          !s.ok()) {
        LOG(ERROR) << s.status();
        return s.status();
      }
      letter_path.pop_back();
    }
  }

  return Dict(std::move(trie), std::move(words));
}

bool Dict::contains(absl::string_view word) const {
  LetterCount lc(word);
  return words_.contains(lc) && words_.at(lc).contains(word);
}

Dict::Dict(const Trie& trie, const absl::flat_hash_set<std::string>& words)
    : trie_(trie) {
  for (const std::string& word : words) words_[LetterCount(word)].insert(word);
}

absl::btree_set<std::string, Dict::LongerStrComp> Dict::WordsMatchingParameters(
    const SearchParameters& params) const {
  absl::btree_set<std::string, LongerStrComp> matches;

  for (const auto& [letter_count, anagrams] : words_) {
    const int len = letter_count.size();
    if (len < params.min_length) continue;
    if (len > params.max_length) continue;
    if (!letter_count.contains(params.letter_subset)) continue;
    if (!params.letter_superset.empty() &&
        !params.letter_superset.contains(letter_count))
      continue;

    for (const std::string& word : anagrams) {
      if (!params.matching_regex.empty() &&
          !RE2::FullMatch(word, params.matching_regex))
        continue;
      matches.insert(word);
    }
  }
  return matches;
}

}  // namespace puzzmo::spelltower