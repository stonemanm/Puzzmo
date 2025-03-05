#include "bongo_gamestate.h"

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

TEST(BongoGameStateTest, Constructor) {
  BongoGameState bgs(kDummyBoard, kLetterValues, LetterCount("aaaabbbcdefgh"));
  EXPECT_EQ(bgs.values(), kLetterValues);
  EXPECT_EQ(bgs.letter_board(), kEmptyBoard);
  EXPECT_EQ(bgs.letter_pool().CharsInOrder(), "aaaabbbcdefgh");
}

TEST(BongoGameStateTest, FillAndClear) {
  BongoGameState bgs(
      kDummyBoard, kLetterValues,
      LetterCount("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbbcdefghi"),
      kEmptyBoard);
  Point p1 = {1, 1};
  Point p2 = {2, 2};
  Point p3 = {3, 3};
  Point p4 = {4, 4};
  Point ip1 = {-1, 2};
  Point ip2 = {2, 5};

  std::vector<Point> path = {{2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}};

  /**
   * FillSquare
   */

  // Uses letter from letter count
  EXPECT_EQ(bgs.letter_pool().count('a'), 37);
  EXPECT_THAT(bgs.FillSquare(p1, 'a'), IsOk());
  EXPECT_EQ(bgs.letter_pool().count('a'), 36);
  EXPECT_EQ(bgs.char_at(p1), 'a');

  // Removes placed letter if present
  EXPECT_THAT(bgs.FillSquare(p2, 'a'), IsOk());
  EXPECT_EQ(bgs.letter_pool().count('a'), 35);
  EXPECT_THAT(bgs.FillSquare(p2, 'b'), IsOk());
  EXPECT_EQ(bgs.letter_pool().count('a'), 36);
  EXPECT_EQ(bgs.char_at(p2), 'b');

  // Fails for letter not in bgs
  EXPECT_THAT(bgs.FillSquare(p3, 'c'), IsOk());
  EXPECT_THAT(bgs.FillSquare(p3, 'z'),
              StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_EQ(bgs.char_at(p3), 'c');

  // Fails if square is locked
  bgs.set_is_locked_at(p4, true);
  EXPECT_EQ(bgs.letter_pool().count('d'), 1);
  EXPECT_THAT(bgs.FillSquare(p4, 'd'),
              StatusIs(absl::StatusCode::kFailedPrecondition));
  EXPECT_EQ(bgs.letter_pool().count('d'), 0);
  EXPECT_EQ(bgs.char_at(p4), '_');

  // Fails if square is out of bounds
  EXPECT_THAT(bgs.FillSquare(ip1, 'a'),
              StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_THAT(bgs.FillSquare(ip2, 'a'),
              StatusIs(absl::StatusCode::kInvalidArgument));

  /**
   * FillPath
   */

  // Fails if given too few characters
  EXPECT_THAT(bgs.FillPath(path, "aaa"),
              StatusIs(absl::StatusCode(absl::StatusCode::kInvalidArgument)));
  EXPECT_EQ(bgs.path_string(path), "__b__");

  // Fails if given too many characters
  EXPECT_THAT(bgs.FillPath(path, "aaaaaaaa"),
              StatusIs(absl::StatusCode(absl::StatusCode::kInvalidArgument)));
  EXPECT_EQ(bgs.path_string(path), "__b__");

  // Fails if point on path locked and doesn't align with the word being placed
  bgs.set_is_locked_at(p2, true);
  EXPECT_THAT(bgs.FillPath(path, "aaaaa"),
              StatusIs(absl::StatusCode::kFailedPrecondition));
  EXPECT_EQ(bgs.path_string(path), "aab__");
  // NOTE: places letters until it reaches the failure.

  // Succeeds even if point locked, if it matches
  // Overwrites tiles that aren't matched.
  EXPECT_THAT(bgs.FillSquare(path[1], 'e'), IsOk());
  EXPECT_EQ(bgs.path_string(path), "aeb__");
  EXPECT_THAT(bgs.FillPath(path, "aabaa"), IsOk());
  EXPECT_EQ(bgs.path_string(path), "aabaa");

  /**
   * ClearSquare
   */

  EXPECT_THAT(bgs.ClearSquare(p1), IsOk());
  EXPECT_EQ(bgs.char_at(p1), '_');

  // Fails if square locked
  EXPECT_THAT(bgs.ClearSquare(p2),
              StatusIs(absl::StatusCode::kFailedPrecondition));
  EXPECT_EQ(bgs.char_at(p2), 'b');

  // Succeeds if square already empty
  bgs.set_is_locked_at(p4, false);
  EXPECT_THAT(bgs.ClearSquare(p4), IsOk());
  EXPECT_EQ(bgs.char_at(p4), '_');

  /**
   * ClearPath
   */

  // Ignores already blank spaces, even if locked
  EXPECT_THAT(bgs.ClearPath({p1, p4}), IsOk());

  // Succeeds but skips locked spaces
  bgs.set_is_locked_at(p2, false);
  bgs.set_is_locked_at(path[1], true);
  bgs.set_is_locked_at(path[4], true);
  EXPECT_THAT(bgs.ClearPath(path), IsOk());
  EXPECT_EQ(bgs.path_string(path), "_a__a");

  // Fails if path contains invalid point, but does the work before that.
  bgs.set_is_locked_at(path[4], false);
  EXPECT_THAT(bgs.ClearPath({path[4], ip1}),
              StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_EQ(bgs.char_at(path[4]), '_');

  /**
   * ClearBoard
   */

  // Locked tiles survive
  EXPECT_EQ(bgs.char_at(p3), 'c');
  EXPECT_EQ(bgs.char_at(path[1]), 'a');
  EXPECT_THAT(bgs.ClearBoard(), IsOk());
  EXPECT_EQ(bgs.char_at(p3), '_');
  EXPECT_EQ(bgs.char_at(path[1]), 'a');
}

TEST(BongoGameStateTest, LowercasePathMethods) {
  BongoGameState bgs(kDummyBoard, kLetterValues, LetterCount("djmpv"),
                     {"abc_e", "fghi_", "kl_no", "_qrst", "u_wxy"});
  EXPECT_EQ(bgs.path_string(bgs.row_path(0)), "abc_e");
  EXPECT_EQ(bgs.path_string(bgs.row_path(1)), "fghi_");
  EXPECT_EQ(bgs.path_string(bgs.row_path(2)), "kl_no");
  EXPECT_EQ(bgs.path_string(bgs.row_path(3)), "_qrst");
  EXPECT_EQ(bgs.path_string(bgs.row_path(4)), "u_wxy");
  EXPECT_EQ(bgs.path_string(bgs.bonus_path()), "ag_s");
}

TEST(BongoGameStateTest, GetWord) {
  BongoGameState bgs(kDummyBoard, kLetterValues, LetterCount("djmpv"),
                     {"abc_e", "fghi_", "kl_no", "_qrst", "u_wxy"});
  EXPECT_EQ(bgs.GetWord(bgs.row_path(0)), "abc");
  EXPECT_EQ(bgs.GetWord(bgs.row_path(1)), "fghi");
  EXPECT_EQ(bgs.GetWord(bgs.row_path(2)), "");
  EXPECT_EQ(bgs.GetWord(bgs.row_path(3)), "qrst");
  EXPECT_EQ(bgs.GetWord(bgs.row_path(4)), "wxy");
  EXPECT_EQ(bgs.GetWord(bgs.bonus_path()), "");
  ASSERT_THAT(bgs.FillSquare({2, 2}, 'm'), IsOk());
  EXPECT_EQ(bgs.GetWord(bgs.bonus_path()), "agms");
}

TEST(BongoGameStateTest, IsComplete) {
  BongoGameState bgs(kDummyBoard, kLetterValues, LetterCount("djmpv"),
                     {"abc_e", "fghi_", "kl_no", "pqr_t", "u_wxy"});
  EXPECT_FALSE(bgs.IsComplete());
  ASSERT_THAT(bgs.FillSquare({2, 2}, 'm'), IsOk());
  // Now every row has a word, even though the bonus does not
  EXPECT_TRUE(bgs.IsComplete());
}

TEST(BongoGameStateTest, MostRestrictedWordlessRow) {
  BongoGameState bgs(kDummyBoard, kLetterValues,
                     LetterCount("abcdefghijklmnopqrstuvwxy"));
  // All else equal, defaults to the lowest index
  EXPECT_EQ(bgs.MostRestrictedWordlessRow(), 0);

  // Adding a letter makes it default to that option
  ASSERT_THAT(bgs.FillSquare({1, 0}, 'f'), IsOk());
  EXPECT_EQ(bgs.MostRestrictedWordlessRow(), 1);

  // If a row has 5 letters, it's not considered
  ASSERT_THAT(bgs.FillPath(bgs.row_path(1), "fghij"), IsOk());
  EXPECT_EQ(bgs.MostRestrictedWordlessRow(), 0);

  // Two rows with three letters each, but one has a subsequence, so it's
  // discounted
  ASSERT_THAT(bgs.FillPath({{2, 1}, {2, 2}, {2, 3}}, "lmn"), IsOk());
  ASSERT_THAT(bgs.FillPath({{3, 0}, {3, 2}, {3, 4}}, "prt"), IsOk());
  EXPECT_EQ(bgs.MostRestrictedWordlessRow(), 3);
}

TEST(BongoGameStateTest, MultiplierSquares) {
  BongoGameState bgs(kDummyBoard, kLetterValues, LetterCount(""));
  Point p0 = {0, 4};
  Point p1 = {1, 4};
  Point p2 = {2, 4};
  EXPECT_THAT(bgs.MultiplierSquares(), ::testing::ElementsAre(p0, p1, p2));
}

TEST(BongoGameStateTest, NMostValuableTiles) {
  BongoGameState bgs(kDummyBoard, kLetterValues, LetterCount("afggh"),
                     kEmptyBoard);
  EXPECT_EQ(bgs.NMostValuableTiles(0), "");
  EXPECT_EQ(bgs.NMostValuableTiles(1), "h");
  EXPECT_EQ(bgs.NMostValuableTiles(2), "hg");
  EXPECT_EQ(bgs.NMostValuableTiles(3), "hgg");
  EXPECT_EQ(bgs.NMostValuableTiles(4), "hggf");
  EXPECT_EQ(bgs.NMostValuableTiles(5), "hggfa");
  EXPECT_EQ(bgs.NMostValuableTiles(6), "hggfa");
}

TEST(BongoGameStateTest, RegexForPath) {
  BongoGameState bgs(kDummyBoard, kLetterValues, LetterCount("jsmmmmmmmmmm"),
                     {"abcde", "fghi_", "_____", "pqr_t", "_____"});
  EXPECT_EQ(bgs.RegexForPath(bgs.row_path(0)), "abcde");
  EXPECT_EQ(bgs.RegexForPath(bgs.row_path(1)), "fghi[jms]");
  EXPECT_EQ(bgs.RegexForPath(bgs.row_path(2)), "");
  EXPECT_EQ(bgs.RegexForPath(bgs.row_path(3)), "pqr[jms]t");
  EXPECT_EQ(bgs.RegexForPath(bgs.bonus_path()), "ag[jms][jms]");
}

TEST(BongoGameStateTest, AllOrNumLetters) {
  const LetterCount all_letters("aaaabbbcdefgh");
  BongoGameState bgs(kDummyBoard, kLetterValues, all_letters);
  EXPECT_EQ(bgs.AllLetters(), all_letters);
  EXPECT_EQ(bgs.NumLetters(), 13);
  EXPECT_EQ(bgs.NumLettersLeft(), 13);
  EXPECT_EQ(bgs.NumLettersPlaced(), 0);

  ASSERT_THAT(bgs.FillPath(bgs.bonus_path(), "abcd"), IsOk());
  EXPECT_EQ(bgs.AllLetters(), all_letters);
  EXPECT_EQ(bgs.NumLetters(), 13);
  EXPECT_EQ(bgs.NumLettersLeft(), 9);
  EXPECT_EQ(bgs.NumLettersPlaced(), 4);
}

TEST(BongoGameStateTest, IsChildOf) {
  BongoGameState start(kDummyBoard, kLetterValues,
                       LetterCount("abcdefghijklmnopqrstuvwxy"));
  BongoGameState partial(kDummyBoard, kLetterValues,
                         LetterCount("bcdefhijklnopqrtuvwxy"),
                         {"a____", "_g___", "__m__", "___s_", "_____"});
  BongoGameState end(kDummyBoard, kLetterValues, LetterCount(),
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
  BongoGameState partial_alternate(
      kDummyBoard, kLetterValues, LetterCount("abcdfghiklmnpqrstuvwxy"),
      {"____e", "____j", "____o", "_____", "_____"});
  EXPECT_FALSE(partial.IsChildOf(partial_alternate));
  EXPECT_FALSE(partial_alternate.IsChildOf(partial));

  // Different starting letters returns false.
  BongoGameState different_letters(kDummyBoard, kLetterValues,
                                   LetterCount("zzzzzzzzzz"));
  EXPECT_FALSE(start.IsChildOf(different_letters));
  EXPECT_FALSE(different_letters.IsChildOf(start));

  // Different mult boards returns false.
  BongoGameState different_multipliers(
      {"*____", "_*___", "__*__", "___*_", "223__"}, kLetterValues,
      LetterCount("abcdefghijklmnopqrstuvwxy"));
  EXPECT_FALSE(start.IsChildOf(different_multipliers));
  EXPECT_FALSE(different_multipliers.IsChildOf(start));

  // Different bonus path returns false.
  BongoGameState different_bonus_path(
      {"___*2", "__*_2", "_*__3", "*____", "_____"}, kLetterValues,
      LetterCount("abcdefghijklmnopqrstuvwxy"));
  EXPECT_FALSE(start.IsChildOf(different_bonus_path));
  EXPECT_FALSE(different_bonus_path.IsChildOf(start));

  // Different values returns false.
  BongoGameState different_values(
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