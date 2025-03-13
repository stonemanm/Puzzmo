#include "dict.h"

#include "absl/status/status_matchers.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace puzzmo::bongo {
namespace {

TEST(DictTest, LoadFromFiles) {
  absl::StatusOr<Dict> dict = Dict::LoadFromFiles();
  EXPECT_THAT(dict, absl_testing::IsOk());
}

TEST(DictTest, IsCommonOrValidWord) {
  absl::flat_hash_set<std::string> valid_words = {"monkey", "panel", "vines",
                                                  "flute", "finds"};
  absl::flat_hash_set<std::string> common_words = {"panel", "flute", "finds"};
  Dict dict(std::move(valid_words), std::move(common_words));

  EXPECT_FALSE(dict.contains(""));
  EXPECT_FALSE(dict.IsCommonWord(""));

  EXPECT_TRUE(dict.contains("monkey"));
  EXPECT_FALSE(dict.IsCommonWord("monkey"));

  EXPECT_TRUE(dict.contains("panel"));
  EXPECT_TRUE(dict.IsCommonWord("panel"));
}

TEST(DictTest, GetMatchingWords) {
  absl::flat_hash_set<std::string> valid_words = {"monkey", "panel", "vines",
                                                  "flute", "finds"};
  absl::flat_hash_set<std::string> common_words = {"panel", "flute", "finds"};
  Dict dict(std::move(valid_words), std::move(common_words));

  EXPECT_TRUE(dict.WordsMatchingParameters({}).contains("flute"));

  EXPECT_TRUE(
      dict.WordsMatchingParameters({.min_length = 5}).contains("flute"));
  EXPECT_FALSE(
      dict.WordsMatchingParameters({.min_length = 6}).contains("flute"));

  EXPECT_TRUE(
      dict.WordsMatchingParameters({.max_length = 5}).contains("flute"));
  EXPECT_FALSE(
      dict.WordsMatchingParameters({.max_length = 4}).contains("flute"));

  EXPECT_TRUE(dict.WordsMatchingParameters({.min_letters = LetterCount("ef")})
                  .contains("flute"));
  EXPECT_FALSE(dict.WordsMatchingParameters({.min_letters = LetterCount("gh")})
                   .contains("flute"));

  EXPECT_TRUE(
      dict.WordsMatchingParameters({.max_letters = LetterCount("aflutter")})
          .contains("flute"));
  EXPECT_FALSE(
      dict.WordsMatchingParameters({.max_letters = LetterCount("atwitter")})
          .contains("flute"));

  EXPECT_TRUE(dict.WordsMatchingParameters({.matching_regex = "fi..."})
                  .contains("finds"));
  EXPECT_FALSE(dict.WordsMatchingParameters({.matching_regex = "fi[aeiou].."})
                   .contains("finds"));
}

}  // namespace
}  // namespace puzzmo::bongo