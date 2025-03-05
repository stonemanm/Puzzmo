#include "dict.h"

#include "absl/status/status_matchers.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace puzzmo::bongo {
namespace {

TEST(DictTest, LoadFromFiles) {
  auto dict = Dict::LoadFromFiles();
  EXPECT_THAT(dict, absl_testing::IsOk());
}

TEST(DictTest, IsCommonOrValidWord) {
  absl::flat_hash_set<std::string> valid_words = {"monkey", "panel", "vines",
                                                  "flute", "finds"};
  absl::flat_hash_set<std::string> common_words = {"panel", "flute", "finds"};
  Dict::SearchableWords sw = {
      {{.length = 6, .letters = LetterCount("monkey")}, {"monkey"}},
      {{.length = 5, .letters = LetterCount("panel")}, {"panel"}},
      {{.length = 5, .letters = LetterCount("vines")}, {"vines"}},
      {{.length = 5, .letters = LetterCount("flute")}, {"flute"}},
      {{.length = 5, .letters = LetterCount("finds")}, {"finds"}},
  };
  Dict dict(std::move(valid_words), std::move(common_words), std::move(sw));

  EXPECT_FALSE(dict.IsCommonWord(""));
  EXPECT_FALSE(dict.IsValidWord(""));

  EXPECT_FALSE(dict.IsCommonWord("monkey"));
  EXPECT_TRUE(dict.IsValidWord("monkey"));

  EXPECT_TRUE(dict.IsCommonWord("panel"));
  EXPECT_TRUE(dict.IsValidWord("panel"));
}

TEST(DictTest, GetMatchingWords) {
  absl::flat_hash_set<std::string> valid_words = {"monkey", "panel", "vines",
                                                  "flute", "finds"};
  absl::flat_hash_set<std::string> common_words = {"panel", "flute", "finds"};
  Dict::SearchableWords sw = {
      {{.length = 6, .letters = LetterCount("monkey")}, {"monkey"}},
      {{.length = 5, .letters = LetterCount("panel")}, {"panel"}},
      {{.length = 5, .letters = LetterCount("vines")}, {"vines"}},
      {{.length = 5, .letters = LetterCount("flute")}, {"flute"}},
      {{.length = 5, .letters = LetterCount("finds")}, {"finds"}},
  };
  Dict dict(std::move(valid_words), std::move(common_words), std::move(sw));

  EXPECT_TRUE(dict.GetMatchingWords({}).contains("flute"));

  EXPECT_TRUE(dict.GetMatchingWords({.min_length = 5}).contains("flute"));
  EXPECT_FALSE(dict.GetMatchingWords({.min_length = 6}).contains("flute"));

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