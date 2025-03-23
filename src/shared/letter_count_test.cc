#include "letter_count.h"

#include "absl/status/status.h"
#include "absl/status/status_matchers.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace puzzmo {
namespace {

using ::absl_testing::IsOk;
using ::absl_testing::StatusIs;
using ::testing::StrEq;

TEST(LetterCountTest, EmptyConstructor) {
  LetterCount lc;
  EXPECT_TRUE(lc.empty());
  EXPECT_EQ(lc.RegexMatchingContents(), "[]");
  EXPECT_EQ(lc.CharsInOrder(), "");
}

TEST(LetterCountTest, StringConstructor) {
  LetterCount lc("Can you hear me?");
  EXPECT_EQ(lc.count('a'), 2);
  EXPECT_EQ(lc.count('b'), 0);
  EXPECT_EQ(lc.count('c'), 1);
  EXPECT_EQ(lc.count('d'), 0);
  EXPECT_EQ(lc.count('e'), 2);
  EXPECT_EQ(lc.count('f'), 0);
  EXPECT_EQ(lc.count('g'), 0);
  EXPECT_EQ(lc.count('h'), 1);
  EXPECT_EQ(lc.count('i'), 0);
  EXPECT_EQ(lc.count('j'), 0);
  EXPECT_EQ(lc.count('k'), 0);
  EXPECT_EQ(lc.count('l'), 0);
  EXPECT_EQ(lc.count('m'), 1);
  EXPECT_EQ(lc.count('n'), 1);
  EXPECT_EQ(lc.count('o'), 1);
  EXPECT_EQ(lc.count('p'), 0);
  EXPECT_EQ(lc.count('q'), 0);
  EXPECT_EQ(lc.count('r'), 1);
  EXPECT_EQ(lc.count('s'), 0);
  EXPECT_EQ(lc.count('t'), 0);
  EXPECT_EQ(lc.count('u'), 1);
  EXPECT_EQ(lc.count('v'), 0);
  EXPECT_EQ(lc.count('w'), 0);
  EXPECT_EQ(lc.count('x'), 0);
  EXPECT_EQ(lc.count('y'), 1);
  EXPECT_EQ(lc.count('z'), 0);
  EXPECT_EQ(lc.count('?'), 0);
}

TEST(LetterCountTest, AddLetter) {
  LetterCount lc;
  EXPECT_THAT(lc.AddLetter('a'), IsOk());
  EXPECT_THAT(lc.AddLetter('a'), IsOk());
  EXPECT_THAT(lc.AddLetter('b', 3), IsOk());
  EXPECT_THAT(lc.AddLetter('b', 2), IsOk());
  EXPECT_THAT(lc.AddLetter('c', 0), IsOk());
  EXPECT_THAT(lc.AddLetter('d', -1),
              StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(lc.AddLetter('?'), StatusIs(absl::StatusCode::kInvalidArgument));
}

TEST(LetterCountTest, CharsInOrder) {
  LetterCount lc("Can you hear me?");
  EXPECT_EQ(lc.CharsInOrder(), "aaceehmnoruy");
}

TEST(LetterCountTest, Count) {
  LetterCount lc("Can you hear me?");
  EXPECT_EQ(lc.count('a'), 2);
  EXPECT_EQ(lc['a'], 2);
  EXPECT_EQ(lc.count('b'), 0);
  EXPECT_EQ(lc['b'], 0);
  EXPECT_EQ(lc.count('c'), 1);
  EXPECT_EQ(lc['c'], 1);
  EXPECT_EQ(lc.count('?'), 0);
}

TEST(LetterCountTest, RemoveLetter) {
  LetterCount lc("Can you hear me?");

  // There are 2 `a`s.
  EXPECT_THAT(lc.RemoveLetter('a'), IsOk());
  EXPECT_THAT(lc.RemoveLetter('a'), IsOk());
  EXPECT_THAT(lc.RemoveLetter('a'),
              StatusIs(absl::StatusCode::kInvalidArgument));

  // There are 2 `e`s.
  EXPECT_THAT(lc.RemoveLetter('e', 2), IsOk());

  // There is 1 `o`.
  EXPECT_THAT(lc.RemoveLetter('o', 2),
              StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(lc.RemoveLetter('o'), IsOk());

  EXPECT_THAT(lc.RemoveLetter('?'),
              StatusIs(absl::StatusCode::kInvalidArgument));
}

TEST(LetterCountTest, Size) {
  LetterCount lc("Can you hear me?");
  EXPECT_EQ(lc.size(), 12);
}

TEST(LetterCountTest, AbslStringify) {
  LetterCount lc("Can you hear me?");
  EXPECT_EQ(absl::StrFormat("%v", lc),
            "[a:2, c:1, e:2, h:1, m:1, n:1, o:1, r:1, u:1, y:1]");
}

TEST(LetterCountTest, CombinationsOfSize) {
  LetterCount lc("wwwxxyz");
  EXPECT_THAT(lc.CombinationsOfSize(0),
              testing::UnorderedElementsAre(StrEq("")));
  EXPECT_THAT(lc.CombinationsOfSize(1),
              testing::UnorderedElementsAre(StrEq("w"), StrEq("x"), StrEq("y"),
                                            StrEq("z")));
  EXPECT_THAT(lc.CombinationsOfSize(4),
              testing::UnorderedElementsAre(
                  StrEq("wwwx"), StrEq("wwwy"), StrEq("wwwz"), StrEq("wwxx"),
                  StrEq("wwxy"), StrEq("wwxz"), StrEq("wwyz"), StrEq("wxxy"),
                  StrEq("wxxz"), StrEq("wxyz"), StrEq("xxyz")));
}

TEST(LetterCountTest, Contains) {
  LetterCount lc("wwwxxyz");
  EXPECT_TRUE(lc.contains(""));
  EXPECT_TRUE(lc.contains("w"));
  EXPECT_TRUE(lc.contains("wx"));
  EXPECT_TRUE(lc.contains("ww"));
  EXPECT_TRUE(lc.contains("www"));
  EXPECT_FALSE(lc.contains("wwww"));

  LetterCount other("w");
  EXPECT_TRUE(lc.contains(other));
  EXPECT_FALSE(other.contains(lc));
  EXPECT_TRUE(lc.contains(lc));
}

TEST(LetterCountTest, AnyCharRegex) {
  LetterCount lc("zwvwwxxy");
  EXPECT_THAT(lc.RegexMatchingContents(), StrEq("[vwxyz]"));
}

}  // namespace
}  // namespace puzzmo