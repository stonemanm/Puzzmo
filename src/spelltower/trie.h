// -----------------------------------------------------------------------------
// File: trie.h
// -----------------------------------------------------------------------------
//
// This header file defines a trie: the data structure used to store legal words
// in Spelltower for ease of lookup and access.

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

constexpr char kNodeIsWord = '!';
constexpr char kEndOfNode = ']';

// spelltower::TrieNode
//
// A basic implementation of a node in a `Trie`. A `TrieNode` holds 26 pointers
// to children, where for the lowercase letter represented by `char c`,
// `children[c-'a']` provides access to the appropriate child node. Iff the path
// from the root node to this node creates a valid word, `is_word` will be
// `true`. Additionally, a `TrieNode` keeps track of how many of its children,
// including itself, have `is_word == true`.
struct TrieNode {
  std::vector<std::shared_ptr<TrieNode>> children;
  bool is_word = false;
  int words_with_prefix = 0;
  TrieNode() : children(26) {}
};

// spelltower::SerializeTrieNode()
//
// A recursive method to serialize the contents of a `TrieNode` and its
// children to a string.
std::string SerializeTrieNode(const std::shared_ptr<TrieNode>& node);

// spelltower::Trie
//
// A class that holds the root `TrieNode` of the data structure. It provides
// methods to access, mutate, and traverse the trie.
class Trie {
 public:
  //--------------
  // Constructors

  // Creates an empty `Trie`.
  Trie() = default;

  // If constructed from a vector of words, the words are inserted one by one
  // into an empty `Trie`.
  explicit Trie(const std::vector<std::string>& words);

  // If constructed from the string serialization of a trie, the resulting
  // `Trie` will simply be the deserialized form of that trie.
  explicit Trie(absl::string_view serialized_trie);

  // A static method that creates a `Trie` by loading from a file that contains
  // the serialization of a trie.
  static absl::StatusOr<Trie> LoadFromSerializedTrie();

  //-----------
  // Accessors

  // Trie::root()
  //
  // Returns a pointer to the root `TrieNode`.
  std::shared_ptr<TrieNode> root() const;

  // Trie::contains()
  //
  // Returns `true` if the trie contains the word.
  bool contains(absl::string_view word) const;

  // Trie::NumWordsWithPrefix()
  //
  // Follows `prefix` to a `TrieNode`, where it returns
  // `TrieNode::words_with_prefix`. If the prefix is not in the trie, returns 0.
  int NumWordsWithPrefix(absl::string_view prefix) const;

  // Trie::WordsWithPrefix()
  //
  // Follows `prefix` to a `TrieNode`, from which DFS is performed to obtain all
  // possible words. If the prefix is not in the trie, returns an empty set. Can
  // be passed an empty string to return all words in the trie.
  absl::flat_hash_set<std::string> WordsWithPrefix(
      absl::string_view prefix) const;

  //----------
  // Mutators

  // Trie::insert()
  //
  // For each word, traverses to the appropriate `TrieNode`, creating nodes if
  // nonexistent and incrementing `words_with_prefix` at each node along the
  // way. Sets the final node's `is_word` to `true`.
  void insert(absl::string_view word);
  void insert(const std::vector<std::string>& words);

 private:
  //-----------
  // Traversal

  // Trie::AllWordsUnderNode()
  //
  // A helper method for `WordsWithPrefix()`.
  absl::flat_hash_set<std::string> AllWordsUnderNode(
      std::shared_ptr<TrieNode> node, absl::string_view prefix) const;

  // Trie::TraversalHelperDFS()
  //
  // A recursive method used to walk all branches of a trie.
  void TraversalHelperDFS(std::shared_ptr<TrieNode> node, std::string& prefix,
                          absl::flat_hash_set<std::string> words) const;

  // Trie::WalkPath()
  //
  // A non-recursive method that follows the path from `base_node` and returns
  // the node that it reaches, or `nullptr` if no node is found.
  std::shared_ptr<TrieNode> WalkPath(std::shared_ptr<TrieNode> base_node,
                                     absl::string_view path) const;

  //---------
  // Members

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