#include "bongo_gamestate.h"

#include <vector>

#include "absl/strings/str_cat.h"
#include "gtest/gtest.h"

namespace puzzmo {
namespace {

const std::vector<std::string> kDummyBoard = {" * 2 ", "3 *  ", " 2 * ",
                                              "   * ", "    2"};
const std::vector<std::string> kEmptyBoard(5, std::string(5, '_'));
const absl::flat_hash_map<char, int> kLetterValues = {
    {'a', 1},  {'b', 2},  {'c', 3},  {'d', 4},  {'e', 5},  {'f', 6},  {'g', 7},
    {'h', 8},  {'i', 9},  {'j', 10}, {'k', 11}, {'l', 12}, {'m', 13}, {'n', 14},
    {'o', 15}, {'p', 16}, {'q', 17}, {'r', 18}, {'s', 19}, {'t', 20}, {'u', 21},
    {'v', 22}, {'w', 23}, {'x', 24}, {'y', 25}, {'z', 26}};

TEST(BongoGameStateTest, Constructor) {
  BongoGameState state(kDummyBoard, kLetterValues,
                       LetterCount("aaaabbbcdefgh"));
  EXPECT_EQ(state.letter_values(), kLetterValues);
  EXPECT_EQ(state.tile_grid(), kEmptyBoard);
  EXPECT_EQ(state.tiles_remaining().CharsInOrder(), "aaaabbbcdefgh");
}

TEST(BongoGameStateTest, PlaceTile) {
  BongoGameState state(kDummyBoard, kLetterValues, LetterCount("aaaabbbcdefgh"),
                       kEmptyBoard);
  EXPECT_TRUE(state.PlaceLetter({0, 0}, 'a'));
  EXPECT_FALSE(state.PlaceLetter({0, 0}, 'a'));
  EXPECT_FALSE(state.PlaceLetter({0, 5}, 'a'));
  EXPECT_TRUE(state.PlaceLetter({0, 1}, 'c'));
  EXPECT_FALSE(state.PlaceLetter({0, 2}, 'c'));
}

TEST(BongoGameStateTest, RemoveTile) {
  BongoGameState state(kDummyBoard, kLetterValues, LetterCount("aaaabbbcdefgh"),
                       {"xxx__", "___x_", "__x__", "_____", "_____"});
  EXPECT_EQ(state.tiles_remaining().count('x'), 0);
  EXPECT_TRUE(state.RemoveLetter({0, 0}));
  EXPECT_EQ(state.tiles_remaining().count('x'), 1);
  EXPECT_FALSE(state.RemoveLetter({0, 0}));
  EXPECT_EQ(state.tiles_remaining().count('x'), 1);
}

TEST(BongoGameStateTest, RowAndBonusWordAndScore) {
  BongoGameState state(kDummyBoard, kLetterValues, LetterCount(""),
                       {"_q__a", "panel", "vines", "flute", "finds"});
  EXPECT_EQ(state.GetRowWord(0), "qa");
  EXPECT_EQ(state.GetRowWord(1), "panel");
  EXPECT_EQ(state.GetRowWord(2), "vines");
  EXPECT_EQ(state.GetRowWord(3), "flute");
  EXPECT_EQ(state.GetRowWord(4), "finds");
  EXPECT_EQ(state.GetBonusWord(), "qnet");

  EXPECT_EQ(state.RowWordScore(0), 18);
  EXPECT_EQ(state.RowWordScore(1), 80);
  EXPECT_EQ(state.RowWordScore(2), 78);
  EXPECT_EQ(state.RowWordScore(3), 64);
  EXPECT_EQ(state.RowWordScore(4), 71);
  EXPECT_EQ(state.BonusWordScore(), 56);
}

TEST(BongoGameStateTest, NMostValuableTiles) {
  BongoGameState state(kDummyBoard, kLetterValues, LetterCount("afggh"),
                       kEmptyBoard);
  EXPECT_EQ(state.NMostValuableTiles(0), "");
  EXPECT_EQ(state.NMostValuableTiles(1), "h");
  EXPECT_EQ(state.NMostValuableTiles(2), "hg");
  EXPECT_EQ(state.NMostValuableTiles(3), "hgg");
  EXPECT_EQ(state.NMostValuableTiles(4), "hggf");
  EXPECT_EQ(state.NMostValuableTiles(5), "hggfa");
  EXPECT_EQ(state.NMostValuableTiles(6), "hggfa");
}

TEST(BongoGameStateTest, RowRegex) {
  BongoGameState state(kDummyBoard, kLetterValues, LetterCount("lliiiiiffm"),
                       {"_qat_", "panel", "vines", "flute", "finds"});
  const std::string open = state.tiles_remaining().RegexMatchingChars();
  EXPECT_EQ(state.RegexForRow(0), absl::StrCat(open, "qat", open));
  EXPECT_EQ(state.RegexForRow(1), "panel");
}

}  // namespace
}  // namespace puzzmo