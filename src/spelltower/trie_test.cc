#include "trie.h"

#include <sstream>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace puzzmo::spelltower {
namespace {

TEST(TrieTest, SerializeAndStringify) {
  Trie trie;
  trie.insert("algebra");
  trie.insert("alpaca");
  trie.insert("blpaca");

  const std::string serialized =
      "3a2l2g1e1b1r1a1!]]]]]p1a1c1a1!]]]]]]b1l1p1a1c1a1!]]]]]]]";

  std::stringstream out;
  out << trie;
  EXPECT_EQ(out.str(), serialized);

  EXPECT_EQ(absl::StrFormat("%v", trie), serialized);
}

TEST(TrieTest, Deserialize) {
  Trie trie("3a2l2g1e1b1r1a1!]]]]]p1a1c1a1!]]]]]]b1l1p1a1c1a1!]]]]]]]");
  // EXPECT_EQ(trie.root(), nullptr);
  EXPECT_EQ(trie.root()->words_with_prefix, 3);
}

TEST(TrieTest, WordsWithPrefix) {
  Trie trie("3a2l2g1e1b1r1a1!]]]]]p1a1c1a1!]]]]]]b1l1p1a1c1a1!]]]]]]]");
  EXPECT_THAT(trie.WordsWithPrefix(""),
              testing::UnorderedElementsAre("algebra", "alpaca", "blpaca"));
}

}  // namespace
}  // namespace puzzmo::spelltower