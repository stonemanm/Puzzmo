#include "gamestate.h"

#include <vector>

#include "absl/status/status.h"
#include "absl/status/status_matchers.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace puzzmo::bongo {
namespace {

using ::absl_testing::IsOk;
using ::absl_testing::StatusIs;

const std::vector<std::string> kDummyBoard = {"*___2", "_*__2", "__*_3",
                                              "___*_", "_____"};
const std::vector<std::string> kEmptyBoard(5, std::string(5, '_'));
const absl::flat_hash_map<char, int> kLetterValues = {
    {'a', 1},  {'b', 2},  {'c', 3},  {'d', 4},  {'e', 5},  {'f', 6},  {'g', 7},
    {'h', 8},  {'i', 9},  {'j', 10}, {'k', 11}, {'l', 12}, {'m', 13}, {'n', 14},
    {'o', 15}, {'p', 16}, {'q', 17}, {'r', 18}, {'s', 19}, {'t', 20}, {'u', 21},
    {'v', 22}, {'w', 23}, {'x', 24}, {'y', 25}, {'z', 26}};

TEST(GameStateTest, Constructor) {
  Gamestate bgs(kDummyBoard, kLetterValues, LetterCount("aaaabbbcdefgh"));
  EXPECT_EQ(bgs.letter_values(), kLetterValues);
  EXPECT_EQ(bgs.unplaced_letters().CharsInOrder(), "aaaabbbcdefgh");
}

TEST(GameStateTest, UpperBoundOnScore) {
  Gamestate bgs(kDummyBoard, kLetterValues,
                LetterCount("abcdefghijklmnopqrstuvwxy"));
  EXPECT_EQ(bgs.UpperBoundOnScore(), 679);
}

TEST(GameStateTest, FillAndClear) {
  Gamestate bgs(kDummyBoard, kLetterValues,
                LetterCount("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbbcdefghi"));
  Point p1 = {1, 1};
  Point p2 = {2, 2};
  Point p3 = {3, 3};
  Point p4 = {4, 4};
  Point ip1 = {-1, 2};
  Point ip2 = {2, 5};

  std::vector<Point> line = {{2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}};

  /**
   * FillCell
   */

  // Uses letter from letter count
  EXPECT_EQ(bgs.unplaced_letters().count('a'), 37);
  EXPECT_THAT(bgs.FillCell(p1, 'a'), IsOk());
  EXPECT_EQ(bgs.unplaced_letters().count('a'), 36);
  EXPECT_EQ(bgs[p1].letter, 'a');

  // Removes placed letter if present
  EXPECT_THAT(bgs.FillCell(p2, 'a'), IsOk());
  EXPECT_EQ(bgs.unplaced_letters().count('a'), 35);
  EXPECT_THAT(bgs.FillCell(p2, 'b'), IsOk());
  EXPECT_EQ(bgs.unplaced_letters().count('a'), 36);
  EXPECT_EQ(bgs[p2].letter, 'b');

  // Fails for letter not in bgs
  EXPECT_THAT(bgs.FillCell(p3, 'c'), IsOk());
  EXPECT_THAT(bgs.FillCell(p3, 'z'),
              StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_EQ(bgs[p3].letter, 'c');

  // Fails if cell is locked
  bgs[p4].is_locked = true;
  EXPECT_EQ(bgs.unplaced_letters().count('d'), 1);
  EXPECT_THAT(bgs.FillCell(p4, 'd'),
              StatusIs(absl::StatusCode::kFailedPrecondition));
  EXPECT_EQ(bgs.unplaced_letters().count('d'), 0);
  EXPECT_EQ(bgs[p4].letter, kEmptyCell);

  // Fails if cell is out of bounds
  EXPECT_THAT(bgs.FillCell(ip1, 'a'),
              StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(bgs.FillCell(ip2, 'a'),
              StatusIs(absl::StatusCode::kInvalidArgument));

  /**
   * FillLine
   */

  // Fails if given too few characters
  EXPECT_THAT(bgs.FillLine(line, "aaa"),
              StatusIs(absl::StatusCode(absl::StatusCode::kInvalidArgument)));
  EXPECT_EQ(bgs.LineString(line), "__b__");

  // Fails if given too many characters
  EXPECT_THAT(bgs.FillLine(line, "aaaaaaaa"),
              StatusIs(absl::StatusCode(absl::StatusCode::kInvalidArgument)));
  EXPECT_EQ(bgs.LineString(line), "__b__");

  // Fails if point on line locked and doesn't align with the word being placed
  bgs[p2].is_locked = true;
  EXPECT_THAT(bgs.FillLine(line, "aaaaa"),
              StatusIs(absl::StatusCode::kFailedPrecondition));
  EXPECT_EQ(bgs.LineString(line), "aab__");
  // NOTE: places letters until it reaches the failure.

  // Succeeds even if point locked, if it matches
  // Overwrites tiles that aren't matched.
  EXPECT_THAT(bgs.FillCell(line[1], 'e'), IsOk());
  EXPECT_EQ(bgs.LineString(line), "aeb__");
  EXPECT_THAT(bgs.FillLine(line, "aabaa"), IsOk());
  EXPECT_EQ(bgs.LineString(line), "aabaa");

  /**
   * ClearCell
   */

  EXPECT_THAT(bgs.ClearCell(p1), IsOk());
  EXPECT_EQ(bgs[p1].letter, kEmptyCell);

  // Fails if cell locked
  EXPECT_THAT(bgs.ClearCell(p2),
              StatusIs(absl::StatusCode::kFailedPrecondition));
  EXPECT_EQ(bgs[p2].letter, 'b');

  // Succeeds if cell already empty
  bgs[p4].is_locked = false;
  EXPECT_THAT(bgs.ClearCell(p4), IsOk());
  EXPECT_EQ(bgs[p4].letter, kEmptyCell);

  /**
   * ClearLine
   */

  // Ignores already blank spaces, even if locked
  EXPECT_THAT(bgs.ClearLine({p1, p4}), IsOk());

  // Succeeds but skips locked spaces
  bgs[p2].is_locked = false;
  bgs[line[1]].is_locked = true;
  bgs[line[4]].is_locked = true;
  EXPECT_THAT(bgs.ClearLine(line), IsOk());
  EXPECT_EQ(bgs.LineString(line), "_a__a");

  // Fails if line contains invalid point, but does the work before that.
  bgs[line[4]].is_locked = false;
  EXPECT_THAT(bgs.ClearLine({line[4], ip1}),
              StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_EQ(bgs[line[4]].letter, kEmptyCell);

  /**
   * ClearBoard
   */

  // Locked tiles survive
  EXPECT_EQ(bgs[p3].letter, 'c');
  EXPECT_EQ(bgs[line[1]].letter, 'a');
  EXPECT_THAT(bgs.ClearBoard(), IsOk());
  EXPECT_EQ(bgs[p3].letter, kEmptyCell);
  EXPECT_EQ(bgs[line[1]].letter, 'a');
}

TEST(GamestateTest, LowercaseLineMethods) {
  Gamestate bgs(kDummyBoard, kLetterValues,
                LetterCount("abcdefghijklmnopqrstuvwxy"),
                {"abc_e", "fghi_", "kl_no", "_qrst", "u_wxy"});
  EXPECT_EQ(bgs.LineString(bgs.line(0)), "abc_e");
  EXPECT_EQ(bgs.LineString(bgs.line(1)), "fghi_");
  EXPECT_EQ(bgs.LineString(bgs.line(2)), "kl_no");
  EXPECT_EQ(bgs.LineString(bgs.line(3)), "_qrst");
  EXPECT_EQ(bgs.LineString(bgs.line(4)), "u_wxy");
  EXPECT_EQ(bgs.LineString(bgs.bonus_line()), "ag_s");
}

TEST(GamestateTest, GetWord) {
  Gamestate bgs(kDummyBoard, kLetterValues,
                LetterCount("abcdefghijklmnopqrstuvwxy"),
                {"abc_e", "fghi_", "kl_no", "_qrst", "u_wxy"});
  EXPECT_EQ(bgs.GetWord(bgs.line(0)), "abc");
  EXPECT_EQ(bgs.GetWord(bgs.line(1)), "fghi");
  EXPECT_EQ(bgs.GetWord(bgs.line(2)), "");
  EXPECT_EQ(bgs.GetWord(bgs.line(3)), "qrst");
  EXPECT_EQ(bgs.GetWord(bgs.line(4)), "wxy");
  EXPECT_EQ(bgs.GetWord(bgs.bonus_line()), "");
  ASSERT_THAT(bgs.FillCell({2, 2}, 'm'), IsOk());
  EXPECT_EQ(bgs.GetWord(bgs.bonus_line()), "agms");
}

TEST(GamestateTest, IsComplete) {
  Gamestate bgs(kDummyBoard, kLetterValues,
                LetterCount("abcdefghijklmnopqrstuvwxy"),
                {"abc_e", "fghi_", "kl_no", "pqr_t", "u_wxy"});
  EXPECT_FALSE(bgs.IsComplete());
  ASSERT_THAT(bgs.FillCell({2, 2}, 'm'), IsOk());
  // Now every row has a word, even though the bonus does not
  EXPECT_TRUE(bgs.IsComplete());
}

TEST(GamestateTest, MostRestrictedWordlessRow) {
  Gamestate bgs(kDummyBoard, kLetterValues,
                LetterCount("abcdefghijklmnopqrstuvwxy"));
  // All else equal, defaults to the lowest index
  EXPECT_EQ(bgs.MostRestrictedWordlessRow(), 0);

  // Adding a letter makes it default to that option
  ASSERT_THAT(bgs.FillCell({1, 0}, 'f'), IsOk());
  EXPECT_EQ(bgs.MostRestrictedWordlessRow(), 1);

  // If a row has 5 letters, it's not considered
  ASSERT_THAT(bgs.FillLine(bgs.line(1), "fghij"), IsOk());
  EXPECT_EQ(bgs.MostRestrictedWordlessRow(), 0);

  // Two rows with three letters each, but one has a subsequence, so it's
  // discounted
  ASSERT_THAT(bgs.FillLine({{2, 1}, {2, 2}, {2, 3}}, "lmn"), IsOk());
  ASSERT_THAT(bgs.FillLine({{3, 0}, {3, 2}, {3, 4}}, "prt"), IsOk());
  EXPECT_EQ(bgs.MostRestrictedWordlessRow(), 3);
}

TEST(GamestateTest, MultiplierPoints) {
  Gamestate bgs(kDummyBoard, kLetterValues, LetterCount());
  Point p0 = {0, 4};
  Point p1 = {1, 4};
  Point p2 = {2, 4};
  EXPECT_THAT(bgs.MultiplierPoints(), ::testing::ElementsAre(p0, p1, p2));
  EXPECT_THAT(bgs.DoublePoints(), ::testing::ElementsAre(p0, p1));
  EXPECT_THAT(bgs.TriplePoint(), ::testing::Eq(p2));
}

TEST(GamestateTest, NMostValuableTiles) {
  Gamestate bgs(kDummyBoard, kLetterValues, LetterCount("afgghz"),
                {"___z_", "_____", "_____", "_____", "_____"});
  EXPECT_EQ(bgs.NMostValuableLetters(0), "");
  EXPECT_EQ(bgs.NMostValuableLetters(1), "h");
  EXPECT_EQ(bgs.NMostValuableLetters(2), "hg");
  EXPECT_EQ(bgs.NMostValuableLetters(3), "hgg");
  EXPECT_EQ(bgs.NMostValuableLetters(4), "hggf");
  EXPECT_EQ(bgs.NMostValuableLetters(5), "hggfa");
  EXPECT_EQ(bgs.NMostValuableLetters(6), "hggfa");
}

TEST(GamestateTest, RegexForLine) {
  Gamestate bgs(kDummyBoard, kLetterValues,
                LetterCount("abcdefghipqrtjsmmmmmmmmmm"),
                {"abcde", "fghi_", "_____", "pqr_t", "_____"});
  EXPECT_EQ(bgs.LineRegex(bgs.line(0)), "abcde");
  EXPECT_EQ(bgs.LineRegex(bgs.line(1)), "fghi[jms]");
  EXPECT_EQ(bgs.LineRegex(bgs.line(2)), "");
  EXPECT_EQ(bgs.LineRegex(bgs.line(3)), "pqr[jms]t");
  EXPECT_EQ(bgs.LineRegex(bgs.bonus_line()), "ag[jms][jms]");
}

TEST(GamestateTest, AllOrNumLetters) {
  const LetterCount all_letters("aaaabbbcdefgh");
  Gamestate bgs(kDummyBoard, kLetterValues, all_letters);
  EXPECT_EQ(bgs.letters(), all_letters);
  EXPECT_EQ(bgs.letters().size(), 13);
  EXPECT_EQ(bgs.unplaced_letters().size(), 13);
  EXPECT_EQ(bgs.placed_letters().size(), 0);

  ASSERT_THAT(bgs.FillLine(bgs.bonus_line(), "abcd"), IsOk());
  EXPECT_EQ(bgs.letters(), all_letters);
  EXPECT_EQ(bgs.letters().size(), 13);
  EXPECT_EQ(bgs.unplaced_letters().size(), 9);
  EXPECT_EQ(bgs.placed_letters().size(), 4);
}

TEST(GamestateTest, IsChildOf) {
  const LetterCount letters("abcdefghijklmnopqrstuvwxy");
  Gamestate start(kDummyBoard, kLetterValues, letters);
  Gamestate partial(kDummyBoard, kLetterValues, letters,
                    {"a____", "_g___", "__m__", "___s_", "_____"});
  Gamestate end(kDummyBoard, kLetterValues, letters,
                {"abcde", "fghij", "klmno", "pqrst", "uvwxy"});

  // True if passed a copy of itself
  EXPECT_TRUE(start.IsChildOf(start));
  EXPECT_TRUE(partial.IsChildOf(partial));
  EXPECT_TRUE(end.IsChildOf(end));

  // Unidirectional
  EXPECT_TRUE(partial.IsChildOf(start));
  EXPECT_TRUE(end.IsChildOf(start));
  EXPECT_TRUE(end.IsChildOf(partial));
  EXPECT_FALSE(start.IsChildOf(partial));
  EXPECT_FALSE(start.IsChildOf(end));
  EXPECT_FALSE(partial.IsChildOf(end));

  // Incompatible placed letters returns false.
  Gamestate partial_alternate(kDummyBoard, kLetterValues,
                              LetterCount("abcdfghiklmnpqrstuvwxy"),
                              {"____e", "____j", "____o", "_____", "_____"});
  EXPECT_FALSE(partial.IsChildOf(partial_alternate));
  EXPECT_FALSE(partial_alternate.IsChildOf(partial));

  // Different starting letters returns false.
  Gamestate different_letters(kDummyBoard, kLetterValues,
                              LetterCount("zzzzzzzzzz"));
  EXPECT_FALSE(start.IsChildOf(different_letters));
  EXPECT_FALSE(different_letters.IsChildOf(start));

  // Different mult boards returns false.
  Gamestate different_multipliers({"*____", "_*___", "__*__", "___*_", "223__"},
                                  kLetterValues,
                                  LetterCount("abcdefghijklmnopqrstuvwxy"));
  EXPECT_FALSE(start.IsChildOf(different_multipliers));
  EXPECT_FALSE(different_multipliers.IsChildOf(start));

  // Different bonus line returns false.
  Gamestate different_bonus_line({"___*2", "__*_2", "_*__3", "*____", "_____"},
                                 kLetterValues,
                                 LetterCount("abcdefghijklmnopqrstuvwxy"));
  EXPECT_FALSE(start.IsChildOf(different_bonus_line));
  EXPECT_FALSE(different_bonus_line.IsChildOf(start));

  // Different values returns false.
  Gamestate different_values(
      kDummyBoard,
      {{'a', 1}, {'b', 1}, {'c', 1}, {'d', 1}, {'e', 1}, {'f', 1}, {'g', 1},
       {'h', 1}, {'i', 1}, {'j', 1}, {'k', 1}, {'l', 1}, {'m', 1}, {'n', 1},
       {'o', 1}, {'p', 1}, {'q', 1}, {'r', 1}, {'s', 1}, {'t', 1}, {'u', 1},
       {'v', 1}, {'w', 1}, {'x', 1}, {'y', 1}, {'z', 1}},
      LetterCount("abcdefghijklmnopqrstuvwxy"));
  EXPECT_FALSE(start.IsChildOf(different_values));
  EXPECT_FALSE(different_values.IsChildOf(start));
}

}  // namespace
}  // namespace puzzmo::bongo