#include "dict.h"

#include "absl/status/status_matchers.h"
#include "gtest/gtest.h"

namespace puzzmo::spelltower {
namespace {

TEST(DictTest, LoadDictFromSerializedTrie) {
  absl::StatusOr<Dict> dict = Dict::LoadDictFromSerializedTrie();
  EXPECT_THAT(dict.status(), absl_testing::IsOk());
  EXPECT_EQ(dict->trie().contains("gargantuan"),
            dict->words().at(LetterCount("gargantuan")).contains("gargantuan"));
}

TEST(DictTest, WordsMatchingParameters) {
  Dict dict(Trie({"car", "crab", "crabs", "scarab", "bin", "bind", "binds",
                  "binder", "binders"}));

  EXPECT_THAT(
      dict.WordsMatchingParameters({.min_length = 6}),
      testing::ElementsAre(testing::StrEq("binders"), testing::StrEq("binder"),
                           testing::StrEq("scarab")));
  EXPECT_THAT(
      dict.WordsMatchingParameters({.max_length = 3}),
      testing::ElementsAre(testing::StrEq("bin"), testing::StrEq("car")));
  EXPECT_THAT(
      dict.WordsMatchingParameters({.letter_subset = LetterCount("c")}),
      testing::ElementsAre(testing::StrEq("scarab"), testing::StrEq("crabs"),
                           testing::StrEq("crab"), testing::StrEq("car")));
  EXPECT_THAT(
      dict.WordsMatchingParameters({.letter_superset = LetterCount("abcr")}),
      testing::ElementsAre(testing::StrEq("crab"), testing::StrEq("car")));
  EXPECT_THAT(
      dict.WordsMatchingParameters({.matching_regex = ".*r"}),
      testing::ElementsAre(testing::StrEq("binder"), testing::StrEq("car")));
  EXPECT_THAT(
      dict.WordsMatchingParameters({.min_length = 5, .max_length = 6}),
      testing::ElementsAre(testing::StrEq("binder"), testing::StrEq("scarab"),
                           testing::StrEq("binds"), testing::StrEq("crabs")));
  EXPECT_THAT(
      dict.WordsMatchingParameters(
          {.min_length = 4, .letter_subset = LetterCount("cr")}),
      testing::ElementsAre(testing::StrEq("scarab"), testing::StrEq("crabs"),
                           testing::StrEq("crab")));
  EXPECT_THAT(
      dict.WordsMatchingParameters(
          {.min_length = 5, .letter_superset = LetterCount("bdeinrs")}),
      testing::ElementsAre(testing::StrEq("binders"), testing::StrEq("binder"),
                           testing::StrEq("binds")));
  EXPECT_THAT(
      dict.WordsMatchingParameters(
          {.max_length = 4, .letter_subset = LetterCount("cr")}),
      testing::ElementsAre(testing::StrEq("crab"), testing::StrEq("car")));
  EXPECT_THAT(
      dict.WordsMatchingParameters(
          {.max_length = 5, .letter_superset = LetterCount("bdeinrs")}),
      testing::ElementsAre(testing::StrEq("binds"), testing::StrEq("bind"),
                           testing::StrEq("bin")));
  EXPECT_THAT(
      dict.WordsMatchingParameters({.letter_subset = LetterCount("s"),
                                    .letter_superset = LetterCount("bdeinrs")}),
      testing::ElementsAre(testing::StrEq("binders"), testing::StrEq("binds")));

  EXPECT_THAT(
      dict.WordsMatchingParameters({.min_length = 5, .matching_regex = ".*r"}),
      testing::ElementsAre(testing::StrEq("binder")));
  EXPECT_THAT(
      dict.WordsMatchingParameters({.max_length = 5, .matching_regex = ".*r"}),
      testing::ElementsAre(testing::StrEq("car")));
  EXPECT_THAT(dict.WordsMatchingParameters(
                  {.letter_subset = LetterCount("b"), .matching_regex = ".*r"}),
              testing::ElementsAre(testing::StrEq("binder")));
  EXPECT_THAT(dict.WordsMatchingParameters(
                  {.letter_superset = LetterCount("biinderxlj"),
                   .matching_regex = ".*r"}),
              testing::ElementsAre(testing::StrEq("binder")));
}

}  // namespace
}  // namespace puzzmo::spelltower