#include "letter_count.h"

#include "absl/status/status.h"
#include "absl/status/status_matchers.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace puzzmo {
namespace {

using ::absl_testing::IsOkAndHolds;
using ::absl_testing::StatusIs;
using ::testing::StrEq;

TEST(LetterCountTest, EmptyConstructor) {
  LetterCount lc;
  EXPECT_TRUE(lc.Empty());
  EXPECT_EQ(lc.AnyCharRegex(), "[]");
  EXPECT_EQ(lc.CharsInOrder(), "");
}

TEST(LetterCountTest, StringConstructor) {
  LetterCount lc("Can you hear me?");
  EXPECT_EQ(lc.Count('a'), 2);
  EXPECT_EQ(lc.Count('b'), 0);
  EXPECT_EQ(lc.Count('c'), 1);
  EXPECT_EQ(lc.Count('d'), 0);
  EXPECT_EQ(lc.Count('e'), 2);
  EXPECT_EQ(lc.Count('f'), 0);
  EXPECT_EQ(lc.Count('g'), 0);
  EXPECT_EQ(lc.Count('h'), 1);
  EXPECT_EQ(lc.Count('i'), 0);
  EXPECT_EQ(lc.Count('j'), 0);
  EXPECT_EQ(lc.Count('k'), 0);
  EXPECT_EQ(lc.Count('l'), 0);
  EXPECT_EQ(lc.Count('m'), 1);
  EXPECT_EQ(lc.Count('n'), 1);
  EXPECT_EQ(lc.Count('o'), 1);
  EXPECT_EQ(lc.Count('p'), 0);
  EXPECT_EQ(lc.Count('q'), 0);
  EXPECT_EQ(lc.Count('r'), 1);
  EXPECT_EQ(lc.Count('s'), 0);
  EXPECT_EQ(lc.Count('t'), 0);
  EXPECT_EQ(lc.Count('u'), 1);
  EXPECT_EQ(lc.Count('v'), 0);
  EXPECT_EQ(lc.Count('w'), 0);
  EXPECT_EQ(lc.Count('x'), 0);
  EXPECT_EQ(lc.Count('y'), 1);
  EXPECT_EQ(lc.Count('z'), 0);
  EXPECT_EQ(lc.Count('?'), -1);
}

TEST(LetterCountTest, AddLetter) {
  LetterCount lc;
  EXPECT_THAT(lc.AddLetter('a'), IsOkAndHolds(1));
  EXPECT_THAT(lc.AddLetter('a'), IsOkAndHolds(2));
  EXPECT_THAT(lc.AddLetter('b', 3), IsOkAndHolds(3));
  EXPECT_THAT(lc.AddLetter('b', 2), IsOkAndHolds(5));
  EXPECT_THAT(lc.AddLetter('c', 0),
              StatusIs(absl::StatusCode::kInvalidArgument));
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
  EXPECT_EQ(lc.Count('a'), 2);
  EXPECT_EQ(lc.Count('b'), 0);
  EXPECT_EQ(lc.Count('c'), 1);
  EXPECT_EQ(lc.Count('?'), -1);
}

TEST(LetterCountTest, RemoveLetter) {
  LetterCount lc("Can you hear me?");
  EXPECT_THAT(lc.RemoveLetter('a'), IsOkAndHolds(1));
  EXPECT_THAT(lc.RemoveLetter('a'), IsOkAndHolds(0));
  EXPECT_THAT(lc.RemoveLetter('a'),
              StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(lc.RemoveLetter('e', 2), IsOkAndHolds(0));
  EXPECT_THAT(lc.RemoveLetter('o', 2),
              StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(lc.RemoveLetter('o'), IsOkAndHolds(0));
  EXPECT_THAT(lc.RemoveLetter('?'),
              StatusIs(absl::StatusCode::kInvalidArgument));
}

TEST(LetterCountTest, Size) {
  LetterCount lc("Can you hear me?");
  EXPECT_EQ(lc.Size(), 12);
}

TEST(LetterCountTest, Valid) {
  LetterCount lc;
  EXPECT_TRUE(lc.Valid());
  LetterCount subt("aaaaa");
  lc -= subt;
  EXPECT_FALSE(lc.Valid());
}

TEST(LetterCountTest, AbslStringify) {
  LetterCount lc("Can you hear me?");
  EXPECT_EQ(absl::StrFormat("%v", lc),
            "[a:2, b:0, c:1, d:0, e:2, f:0, g:0, h:1, i:0, j:0, k:0, l:0, m:1, "
            "n:1, o:1, p:0, q:0, r:1, s:0, t:0, u:1, v:0, w:0, x:0, y:1, z:0]");
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
  EXPECT_TRUE(lc.Contains(""));
  EXPECT_TRUE(lc.Contains("w"));
  EXPECT_TRUE(lc.Contains("wx"));
  EXPECT_TRUE(lc.Contains("ww"));
  EXPECT_TRUE(lc.Contains("www"));
  EXPECT_FALSE(lc.Contains("wwww"));

  LetterCount other("w");
  EXPECT_TRUE(lc.Contains(other));
  EXPECT_FALSE(other.Contains(lc));
  EXPECT_TRUE(lc.Contains(lc));
}

TEST(LetterCountTest, AnyCharRegex) {
  LetterCount lc("zwvwwxxy");
  EXPECT_THAT(lc.AnyCharRegex(), StrEq("[vwxyz]"));
}

} // namespace
} // namespace puzzmo