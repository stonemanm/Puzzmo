#ifndef trie_h
#define trie_h

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"

namespace puzzmo::spelltower {

// A basic implementation of a node in a Trie.
struct TrieNode {
  std::vector<std::shared_ptr<TrieNode>> children;
  bool is_word = false;
  int words_with_prefix = 0;
  TrieNode() : children(26) {}
};

std::string SerializeTrieNode(const std::shared_ptr<TrieNode>& node);

class Trie {
 public:
  Trie() {}
  explicit Trie(const std::vector<std::string>& words);
  explicit Trie(absl::string_view serialized_trie);
  static absl::StatusOr<Trie> LoadFromSerializedTrie();

  void insert(const std::vector<std::string>& words);
  void insert(absl::string_view word);
  bool contains(absl::string_view word) const;
  std::shared_ptr<TrieNode> root() const;

  int NumWordsWithPrefix(absl::string_view prefix) const;
  absl::flat_hash_set<std::string> WordsWithPrefix(
      absl::string_view prefix) const;

 private:
  absl::flat_hash_set<std::string> AllWordsUnderNode(
      std::shared_ptr<TrieNode> node, absl::string_view prefix) const;

  void TraversalHelper(std::shared_ptr<TrieNode> node, std::string& prefix,
                       absl::flat_hash_set<std::string> words) const;

  // Returns nullptr if no node found.
  std::shared_ptr<TrieNode> WalkPath(std::shared_ptr<TrieNode> base_node,
                                     absl::string_view path) const;

  std::shared_ptr<TrieNode> root_ = std::make_shared<TrieNode>();

  //------------------
  // Abseil functions

  template <typename Sink>
  friend void AbslStringify(Sink& sink, const Trie& trie) {
    sink.Append(SerializeTrieNode(trie.root_));
  }
};

std::ostream& operator<<(std::ostream& os,
                         const std::shared_ptr<TrieNode>& node);
std::ostream& operator<<(std::ostream& os, const Trie& trie);

}  // namespace puzzmo::spelltower

#endif