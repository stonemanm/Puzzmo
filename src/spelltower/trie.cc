#include "trie.h"

#include <cctype>
#include <stack>

#include "absl/log/log.h"

namespace puzzmo::spelltower {

constexpr char kNodeIsWord = '!';
constexpr char kEndOfNode = ']';

Trie::Trie(const std::vector<std::string>& words) { insert(words); }

void Trie::insert(const std::vector<std::string>& words) {
  for (const std::string& word : words) insert(word);
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

bool Trie::contains(absl::string_view word) const {
  std::shared_ptr<TrieNode> node = WalkPath(root_, word);
  return node && node->is_word;
}

std::shared_ptr<TrieNode> Trie::root() const { return root_; }

int Trie::NumWordsWithPrefix(absl::string_view prefix) const {
  std::shared_ptr<TrieNode> node = WalkPath(root_, prefix);
  return (node == nullptr) ? 0 : node->words_with_prefix;
}

absl::flat_hash_set<std::string> Trie::WordsWithPrefix(
    absl::string_view prefix) const {
  return AllWordsUnderNode(WalkPath(root_, prefix), prefix);
}

absl::flat_hash_set<std::string> Trie::AllWordsUnderNode(
    std::shared_ptr<TrieNode> node, absl::string_view prefix) const {
  absl::flat_hash_set<std::string> words;
  std::string mutable_prefix = std::string(prefix);
  TraversalHelper(node, mutable_prefix, words);
  return words;
}

void Trie::TraversalHelper(std::shared_ptr<TrieNode> node, std::string& prefix,
                           absl::flat_hash_set<std::string> words) const {
  if (node == nullptr) return;
  if (node->is_word) words.insert(prefix);
  for (int i = 0; i < 26; ++i) {
    std::shared_ptr<TrieNode> child = node->children[i];
    if (child == nullptr) continue;
    prefix.push_back('a' + i);
    TraversalHelper(child, prefix, words);
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

std::ostream& operator<<(std::ostream& os, std::shared_ptr<TrieNode> node) {
  if (node == nullptr) return os;
  if (int wds = node->words_with_prefix; wds > 0) os << wds;
  if (node->is_word) os << kNodeIsWord;
  for (int i = 0; i < 26; ++i) {
    std::shared_ptr<TrieNode> child = node->children[i];
    if (child != nullptr) os << (char)('a' + i);
    os << child;
  }
  os << kEndOfNode;
  return os;
}

std::ostream& operator<<(std::ostream& os, const Trie& trie) {
  os << trie.root();
  return os;
}

}  // namespace puzzmo::spelltower