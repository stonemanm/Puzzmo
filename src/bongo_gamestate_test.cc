#include "bongo_gamestate.h"

#include <vector>

#include "absl/log/log.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace puzzmo {
namespace {

const std::vector<std::string> kDummyBoard = {" * 2 ", "3 *  ", " 2 * ",
                                              "   * ", "    2"};
const std::vector<std::vector<char>> kEmptyBoard(5, std::vector<char>(5, ' '));
const absl::flat_hash_map<char, int> kLetterValues = {
    {'a', 1},  {'b', 2},  {'c', 3},  {'d', 4},  {'e', 5},  {'f', 6},  {'g', 7},
    {'h', 8},  {'i', 9},  {'j', 10}, {'k', 11}, {'l', 12}, {'m', 13}, {'n', 14},
    {'o', 15}, {'p', 16}, {'q', 17}, {'r', 18}, {'s', 19}, {'t', 20}, {'u', 21},
    {'v', 22}, {'w', 23}, {'x', 24}, {'y', 25}, {'z', 26}};

TEST(BongoGameStateTest, Constructor) {
  BongoGameState state(kDummyBoard, kLetterValues,
                       LetterCount("aaaabbbcdefgh"));
  EXPECT_EQ(state.LetterValues(), kLetterValues);
  EXPECT_EQ(state.PlacedTiles(), kEmptyBoard);
  EXPECT_EQ(state.RemainingTiles().CharsInOrder(), "aaaabbbcdefgh");
}

TEST(BongoGameStateTest, PlaceTile) {
  BongoGameState state(kDummyBoard, kLetterValues, LetterCount("aaaabbbcdefgh"),
                       kEmptyBoard);
  EXPECT_TRUE(state.PlaceTile({0, 0}, 'a'));
  EXPECT_FALSE(state.PlaceTile({0, 0}, 'a'));
  EXPECT_FALSE(state.PlaceTile({0, 5}, 'a'));
  EXPECT_TRUE(state.PlaceTile({0, 1}, 'c'));
  EXPECT_FALSE(state.PlaceTile({0, 2}, 'c'));
}

TEST(BongoGameStateTest, RemoveTile) {
  BongoGameState state(kDummyBoard, kLetterValues, LetterCount("aaaabbbcdefgh"),
                       {{'x', 'x', 'x', ' ', ' '},
                        {' ', ' ', ' ', 'x', ' '},
                        {' ', ' ', 'x', ' ', ' '},
                        {' ', ' ', ' ', ' ', ' '},
                        {' ', ' ', ' ', ' ', ' '}});
  EXPECT_EQ(state.RemainingTiles().NumLetters('x'), 0);
  EXPECT_TRUE(state.RemoveTile({0, 0}));
  EXPECT_EQ(state.RemainingTiles().NumLetters('x'), 1);
  EXPECT_FALSE(state.RemoveTile({0, 0}));
  EXPECT_EQ(state.RemainingTiles().NumLetters('x'), 1);
}

TEST(BongoGameStateTest, RowAndBonusWordAndScore) {
  BongoGameState state(kDummyBoard, kLetterValues, LetterCount(""),
                       {{' ', 'q', ' ', ' ', ' '},
                        {'p', 'a', 'n', 'e', 'l'},
                        {'v', 'i', 'n', 'e', 's'},
                        {'f', 'l', 'u', 't', 'e'},
                        {'f', 'i', 'n', 'd', 's'}});
  EXPECT_EQ(state.RowWord(0), "q");
  EXPECT_EQ(state.RowWord(1), "panel");
  EXPECT_EQ(state.RowWord(2), "vines");
  EXPECT_EQ(state.RowWord(3), "flute");
  EXPECT_EQ(state.RowWord(4), "finds");
  EXPECT_EQ(state.BonusWord(), "qnet");

  EXPECT_EQ(state.RowWordScore(0), 17);
  EXPECT_EQ(state.RowWordScore(1), 80);
  EXPECT_EQ(state.RowWordScore(2), 78);
  EXPECT_EQ(state.RowWordScore(3), 64);
  EXPECT_EQ(state.RowWordScore(4), 71);
  EXPECT_EQ(state.BonusWordScore(), 56);
}

} // namespace
} // namespace puzzmo