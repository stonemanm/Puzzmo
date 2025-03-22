#include "solver.h"

#include "absl/status/status.h"
#include "absl/status/status_matchers.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace puzzmo::bongo {
namespace {

using absl_testing::IsOk;
using absl_testing::IsOkAndHolds;
using absl_testing::StatusIs;

TEST(SolverTest, Constructor) {
  //
}

TEST(SolverTest, GetWord) {
  // Gamestate bgs(kDummyBoard, kLetterValues,
  //               LetterCount("abcdefghijklmnopqrstuvwxy"),
  //               {"abc_e", "fghi_", "kl_no", "_qrst", "u_wxy"});
  // EXPECT_EQ(bgs.GetWord(bgs.line(0)), "abc");
  // EXPECT_EQ(bgs.GetWord(bgs.line(1)), "fghi");
  // EXPECT_EQ(bgs.GetWord(bgs.line(2)), "");
  // EXPECT_EQ(bgs.GetWord(bgs.line(3)), "qrst");
  // EXPECT_EQ(bgs.GetWord(bgs.line(4)), "wxy");
  // EXPECT_EQ(bgs.GetWord(bgs.bonus_line()), "");
  // ASSERT_THAT(bgs.FillCell({2, 2}, 'm'), IsOk());
  // EXPECT_EQ(bgs.GetWord(bgs.bonus_line()), "agms");
}

TEST(SolverTest, IsComplete) {
  //   Gamestate bgs(kDummyBoard, kLetterValues,
  //                 LetterCount("abcdefghijklmnopqrstuvwxy"),
  //                 {"abc_e", "fghi_", "kl_no", "pqr_t", "u_wxy"});
  //   EXPECT_FALSE(bgs.IsComplete());
  //   ASSERT_THAT(bgs.FillCell({2, 2}, 'm'), IsOk());
  //   // Now every row has a word, even though the bonus does not
  //   EXPECT_TRUE(bgs.IsComplete());
}

TEST(SolverTest, MostRestrictedWordlessRow) {
  // Gamestate bgs(kDummyBoard, kLetterValues,
  //               LetterCount("abcdefghijklmnopqrstuvwxy"));
  // // All else equal, defaults to the lowest index
  // EXPECT_EQ(bgs.MostRestrictedWordlessRow(), 0);

  // // Adding a letter makes it default to that option
  // ASSERT_THAT(bgs.FillCell({1, 0}, 'f'), IsOk());
  // EXPECT_EQ(bgs.MostRestrictedWordlessRow(), 1);

  // // If a row has 5 letters, it's not considered
  // ASSERT_THAT(bgs.FillLine(bgs.line(1), "fghij"), IsOk());
  // EXPECT_EQ(bgs.MostRestrictedWordlessRow(), 0);

  // // Two rows with three letters each, but one has a subsequence, so it's
  // // discounted
  // ASSERT_THAT(bgs.FillLine({{2, 1}, {2, 2}, {2, 3}}, "lmn"), IsOk());
  // ASSERT_THAT(bgs.FillLine({{3, 0}, {3, 2}, {3, 4}}, "prt"), IsOk());
  // EXPECT_EQ(bgs.MostRestrictedWordlessRow(), 3);
}

}  // namespace
}  // namespace puzzmo::bongo