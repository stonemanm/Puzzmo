#include "letter_count.h"

#include <vector>

#include "absl/log/log.h"
#include "absl/status/status.h"
#include "absl/status/status_matchers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "gtest/gtest.h"

namespace puzzmo {
namespace {

using ::absl_testing::IsOkAndHolds;
using ::absl_testing::StatusIs;

TEST(LetterCountTest, EmptyConstructor) {
  LetterCount lc;
  EXPECT_EQ(lc.Size(), 0);
  EXPECT_EQ(lc.CharsInOrder(), "");
}

TEST(LetterCountTest, StringConstructor) {
  LetterCount lc("Can you hear me?");
  EXPECT_EQ(lc.NumLetters('a'), 2);
  EXPECT_EQ(lc.NumLetters('b'), 0);
  EXPECT_EQ(lc.NumLetters('c'), 1);
  EXPECT_EQ(lc.NumLetters('d'), 0);
  EXPECT_EQ(lc.NumLetters('e'), 2);
  EXPECT_EQ(lc.NumLetters('f'), 0);
  EXPECT_EQ(lc.NumLetters('g'), 0);
  EXPECT_EQ(lc.NumLetters('h'), 1);
  EXPECT_EQ(lc.NumLetters('i'), 0);
  EXPECT_EQ(lc.NumLetters('j'), 0);
  EXPECT_EQ(lc.NumLetters('k'), 0);
  EXPECT_EQ(lc.NumLetters('l'), 0);
  EXPECT_EQ(lc.NumLetters('m'), 1);
  EXPECT_EQ(lc.NumLetters('n'), 1);
  EXPECT_EQ(lc.NumLetters('o'), 1);
  EXPECT_EQ(lc.NumLetters('p'), 0);
  EXPECT_EQ(lc.NumLetters('q'), 0);
  EXPECT_EQ(lc.NumLetters('r'), 1);
  EXPECT_EQ(lc.NumLetters('s'), 0);
  EXPECT_EQ(lc.NumLetters('t'), 0);
  EXPECT_EQ(lc.NumLetters('u'), 1);
  EXPECT_EQ(lc.NumLetters('v'), 0);
  EXPECT_EQ(lc.NumLetters('w'), 0);
  EXPECT_EQ(lc.NumLetters('x'), 0);
  EXPECT_EQ(lc.NumLetters('y'), 1);
  EXPECT_EQ(lc.NumLetters('z'), 0);
  EXPECT_EQ(lc.NumLetters('?'), -1);
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

TEST(LetterCountTest, NumLetters) {
  LetterCount lc("Can you hear me?");
  EXPECT_EQ(lc.NumLetters('a'), 2);
  EXPECT_EQ(lc.NumLetters('b'), 0);
  EXPECT_EQ(lc.NumLetters('c'), 1);
  EXPECT_EQ(lc.NumLetters('?'), -1);
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

} // namespace
} // namespace puzzmo