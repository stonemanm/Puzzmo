#include "trie.h"

#include <cctype>
#include <fstream>
#include <stack>

#include "absl/flags/flag.h"
#include "absl/strings/str_cat.h"

ABSL_FLAG(std::string, serialized_dict_path, "data/serialized_trie.txt",
          "Input file containing all legal words for Spelltower, serialized "
          "for ease of loading.");

namespace puzzmo::spelltower {

constexpr char kNodeIsWord = '!';
constexpr char kEndOfNode = ']';

std::string SerializeTrieNode(const std::shared_ptr<TrieNode>& node) {
  std::string s = "";
  if (node == nullptr) return s;
  if (int wds = node->words_with_prefix; wds > 0) absl::StrAppend(&s, wds);
  if (node->is_word) absl::StrAppend(&s, std::string(1, kNodeIsWord));
  for (int i = 0; i < 26; ++i) {
    std::shared_ptr<TrieNode> child = node->children[i];
    if (child != nullptr) absl::StrAppend(&s, std::string(1, 'a' + i));
    absl::StrAppend(&s, SerializeTrieNode(child));
  }
  absl::StrAppend(&s, std::string(1, kEndOfNode));
  return s;
}

Trie::Trie(const std::vector<std::string>& words) { insert(words); }

absl::StatusOr<Trie> Trie::LoadFromSerializedTrie() {
  std::string path = absl::GetFlag(FLAGS_serialized_dict_path);
  std::ifstream file(path);
  if (!file.is_open())
    return absl::InvalidArgumentError(
        absl::StrCat("Error: Could not open ", path));

  std::string serialized_trie_string;
  std::getline(file, serialized_trie_string);
  Trie trie(serialized_trie_string);
  file.close();

  return trie;
}

std::shared_ptr<TrieNode> Trie::root() const { return root_; }

bool Trie::contains(absl::string_view word) const {
  std::shared_ptr<TrieNode> node = WalkPath(root_, word);
  return node && node->is_word;
}

int Trie::NumWordsWithPrefix(absl::string_view prefix) const {
  std::shared_ptr<TrieNode> node = WalkPath(root_, prefix);
  return (node == nullptr) ? 0 : node->words_with_prefix;
}

absl::flat_hash_set<std::string> Trie::WordsWithPrefix(
    absl::string_view prefix) const {
  return AllWordsUnderNode(WalkPath(root_, prefix), prefix);
}

void Trie::insert(absl::string_view word) {
  std::shared_ptr<TrieNode> node = root_;
  ++(node->words_with_prefix);
  for (char c : word) {
    // Go to the next node
    int i = c - 'a';
    if (node->children[i] == nullptr)
      node->children[i] = std::make_shared<TrieNode>();
    node = node->children[i];

    // Count the word we're adding
    ++(node->words_with_prefix);
  }
  node->is_word = true;
}

void Trie::insert(const std::vector<std::string>& words) {
  for (const std::string& word : words) insert(word);
}

absl::flat_hash_set<std::string> Trie::AllWordsUnderNode(
    std::shared_ptr<TrieNode> node, absl::string_view prefix) const {
  absl::flat_hash_set<std::string> words;
  std::string mutable_prefix = std::string(prefix);
  TraversalHelperDFS(node, mutable_prefix, words);
  return words;
}

void Trie::TraversalHelperDFS(std::shared_ptr<TrieNode> node,
                              std::string& prefix,
                              absl::flat_hash_set<std::string> words) const {
  if (node == nullptr) return;
  if (node->is_word) words.insert(prefix);
  for (int i = 0; i < 26; ++i) {
    std::shared_ptr<TrieNode> child = node->children[i];
    if (child == nullptr) continue;
    prefix.push_back('a' + i);
    TraversalHelperDFS(child, prefix, words);
    prefix.pop_back();
  }
}

std::shared_ptr<TrieNode> Trie::WalkPath(std::shared_ptr<TrieNode> base_node,
                                         absl::string_view path) const {
  for (char c : path) {
    int i = c - 'a';
    if (base_node->children[i] == nullptr) return nullptr;
    base_node = base_node->children[i];
  }
  return base_node;
}

Trie::Trie(absl::string_view serialized_trie) {
  std::shared_ptr<TrieNode> node = root_;
  std::stack<std::shared_ptr<TrieNode>> s;
  for (char c : serialized_trie) {
    if (std::isalpha(c)) {
      int i = c - 'a';
      node->children[i] = std::make_shared<TrieNode>();
      s.push(node);
      node = node->children[i];
    } else if (std::isdigit(c)) {
      node->words_with_prefix *= 10;
      node->words_with_prefix += c - '0';
    } else if (c == kNodeIsWord) {
      node->is_word = true;
    } else if (c == kEndOfNode) {
      if (s.empty()) continue;
      node = s.top();
      s.pop();
    }
  }
}

std::ostream& operator<<(std::ostream& os,
                         const std::shared_ptr<TrieNode>& node) {
  os << SerializeTrieNode(node);
  return os;
}

std::ostream& operator<<(std::ostream& os, const Trie& trie) {
  os << SerializeTrieNode(trie.root());
  return os;
}

}  // namespace puzzmo::spelltower