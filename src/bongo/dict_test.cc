#include "dict.h"

#include "absl/status/status_matchers.h"
#include "gtest/gtest.h"

namespace puzzmo::bongo {
namespace {

TEST(DictTest, Init) {
  Dict dict;
  EXPECT_THAT(dict.Init(), absl_testing::IsOk());
}

TEST(DictTest, IsCommonOrValidWord) {
  Dict dict;
  EXPECT_THAT(dict.Init(), absl_testing::IsOk());

  EXPECT_FALSE(dict.IsCommonWord(""));
  EXPECT_FALSE(dict.IsValidWord(""));

  EXPECT_FALSE(dict.IsCommonWord("wrapt"));
  EXPECT_TRUE(dict.IsValidWord("wrapt"));

  EXPECT_TRUE(dict.IsCommonWord("wraps"));
  EXPECT_TRUE(dict.IsCommonWord("wraps"));
}

TEST(DictTest, GetMatchingWords) {
  Dict dict;
  EXPECT_THAT(dict.Init(), absl_testing::IsOk());

  EXPECT_TRUE(dict.GetMatchingWords({}).contains("flute"));

  EXPECT_TRUE(dict.GetMatchingWords({.min_length = 3}).contains("and"));
  EXPECT_FALSE(dict.GetMatchingWords({.min_length = 4}).contains("and"));

  EXPECT_TRUE(dict.GetMatchingWords({.max_length = 5}).contains("flute"));
  EXPECT_FALSE(dict.GetMatchingWords({.max_length = 4}).contains("flute"));

  EXPECT_TRUE(dict.GetMatchingWords({.min_letters = LetterCount("ef")})
                  .contains("flute"));
  EXPECT_FALSE(dict.GetMatchingWords({.min_letters = LetterCount("gh")})
                   .contains("flute"));

  EXPECT_TRUE(dict.GetMatchingWords({.max_letters = LetterCount("aflutter")})
                  .contains("flute"));
  EXPECT_FALSE(dict.GetMatchingWords({.max_letters = LetterCount("atwitter")})
                   .contains("flute"));

  EXPECT_TRUE(
      dict.GetMatchingWords({.matching_regex = "fi..."}).contains("finds"));
  EXPECT_FALSE(dict.GetMatchingWords({.matching_regex = "fi[aeiou].."})
                   .contains("finds"));
}

}  // namespace
}  // namespace puzzmo::bongo