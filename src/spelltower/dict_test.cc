#include "dict.h"

#include "absl/status/status_matchers.h"
#include "gtest/gtest.h"

namespace puzzmo::spelltower {
namespace {

TEST(DictTest, LoadDictFromSerializedTrie) {
  auto dict = Dict::LoadDictFromSerializedTrie();
  EXPECT_THAT(dict.status(), absl_testing::IsOk());
  EXPECT_EQ(dict->trie().contains("gargantuan"),
            dict->words().at(LetterCount("gargantuan")).contains("gargantuan"));
}

TEST(DictTest, WordsMatchingParameters) {
  Dict dict(Trie({"car", "crab", "crabs", "scarab", "bin", "bind", "binds",
                  "binder", "binders"}));

  EXPECT_THAT(dict.WordsMatchingParameters({.min_length = 6}),
              testing::UnorderedElementsAre(testing::StrEq("scarab"),
                                            testing::StrEq("binder"),
                                            testing::StrEq("binders")));
  EXPECT_THAT(dict.WordsMatchingParameters({.max_length = 3}),
              testing::UnorderedElementsAre(testing::StrEq("car"),
                                            testing::StrEq("bin")));
  EXPECT_THAT(dict.WordsMatchingParameters({.letter_subset = LetterCount("c")}),
              testing::UnorderedElementsAre(
                  testing::StrEq("car"), testing::StrEq("crab"),
                  testing::StrEq("crabs"), testing::StrEq("scarab")));
  EXPECT_THAT(
      dict.WordsMatchingParameters({.letter_superset = LetterCount("abcr")}),
      testing::UnorderedElementsAre(testing::StrEq("car"),
                                    testing::StrEq("crab")));
  EXPECT_THAT(dict.WordsMatchingParameters({.min_length = 5, .max_length = 6}),
              testing::UnorderedElementsAre(
                  testing::StrEq("crabs"), testing::StrEq("scarab"),
                  testing::StrEq("binds"), testing::StrEq("binder")));
  EXPECT_THAT(dict.WordsMatchingParameters(
                  {.min_length = 4, .letter_subset = LetterCount("cr")}),
              testing::UnorderedElementsAre(testing::StrEq("crab"),
                                            testing::StrEq("crabs"),
                                            testing::StrEq("scarab")));
  EXPECT_THAT(dict.WordsMatchingParameters(
                  {.min_length = 5, .letter_superset = LetterCount("bdeinrs")}),
              testing::UnorderedElementsAre(testing::StrEq("binds"),
                                            testing::StrEq("binder"),
                                            testing::StrEq("binders")));
  EXPECT_THAT(dict.WordsMatchingParameters(
                  {.max_length = 4, .letter_subset = LetterCount("cr")}),
              testing::UnorderedElementsAre(testing::StrEq("car"),
                                            testing::StrEq("crab")));
  EXPECT_THAT(dict.WordsMatchingParameters(
                  {.max_length = 5, .letter_superset = LetterCount("bdeinrs")}),
              testing::UnorderedElementsAre(testing::StrEq("bin"),
                                            testing::StrEq("bind"),
                                            testing::StrEq("binds")));
  EXPECT_THAT(
      dict.WordsMatchingParameters({.letter_subset = LetterCount("s"),
                                    .letter_superset = LetterCount("bdeinrs")}),
      testing::UnorderedElementsAre(testing::StrEq("binds"),
                                    testing::StrEq("binders")));
}

}  // namespace
}  // namespace puzzmo::spelltower