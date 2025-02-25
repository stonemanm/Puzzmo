#include "trie.h"

#include <sstream>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace puzzmo {
namespace {

TEST(TrieTest, Serialize) {
  Trie trie;
  trie.insert("algebra");
  trie.insert("alpaca");
  trie.insert("blpaca");

  std::stringstream out;
  out << trie;
  EXPECT_EQ(out.str(),
            "3a2l2g1e1b1r1a1!]]]]]p1a1c1a1!]]]]]]b1l1p1a1c1a1!]]]]]]]");
}

TEST(TrieTest, Deserialize) {
  Trie trie("3a2l2g1e1b1r1a1!]]]]]p1a1c1a1!]]]]]]b1l1p1a1c1a1!]]]]]]]");
  // EXPECT_EQ(trie.root(), nullptr);
  EXPECT_EQ(trie.root()->words_with_prefix, 3);
}

}  // namespace
}  // namespace puzzmo