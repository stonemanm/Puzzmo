#include "spelltower_board.h"

#include <vector>

#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "gtest/gtest.h"

namespace puzzmo {
namespace {

TEST(SpelltowerBoardTest, EmptyConstructor) {
  SpelltowerBoard board({});
  EXPECT_EQ(board.NumRows(), 0);
  EXPECT_EQ(board.NumCols(), 0);
  EXPECT_FALSE(board.HasPoint({0, 0}));
  EXPECT_FALSE(board.HasPoint(0, 0));
  EXPECT_EQ(board.At({0, 0}), '*');
}

TEST(SpelltowerBoardTest, At) {
  SpelltowerBoard board({{'a', 'b', 'c'}, {'d', 'e', 'f'}});
  EXPECT_EQ(board.At({0, 0}), 'a');
  EXPECT_EQ(board.At(0, 0), 'a');
  EXPECT_EQ(board.At({1, 2}), 'f');
  EXPECT_EQ(board.At(1, 2), 'f');
  EXPECT_EQ(board.At({-1, 0}), '*');
  EXPECT_EQ(board.At(-1, 0), '*');
  EXPECT_EQ(board.At({0, -1}), '*');
  EXPECT_EQ(board.At(0, -1), '*');
  EXPECT_EQ(board.At({2, 0}), '*');
  EXPECT_EQ(board.At(2, 0), '*');
  EXPECT_EQ(board.At({0, 3}), '*');
  EXPECT_EQ(board.At(0, 3), '*');
}

TEST(SpelltowerBoardTest, HasPoint) {
  SpelltowerBoard board({{'a', 'b', 'c'}, {'d', 'e', 'f'}});
  EXPECT_TRUE(board.HasPoint({0, 0}));
  EXPECT_TRUE(board.HasPoint(0, 0));
  EXPECT_TRUE(board.HasPoint({1, 2}));
  EXPECT_TRUE(board.HasPoint(1, 2));
  EXPECT_FALSE(board.HasPoint({-1, 0}));
  EXPECT_FALSE(board.HasPoint(-1, 0));
  EXPECT_FALSE(board.HasPoint({0, -1}));
  EXPECT_FALSE(board.HasPoint(0, -1));
  EXPECT_FALSE(board.HasPoint({2, 0}));
  EXPECT_FALSE(board.HasPoint(2, 0));
  EXPECT_FALSE(board.HasPoint({0, 3}));
  EXPECT_FALSE(board.HasPoint(0, 3));
}

TEST(SpelltowerBoardTest, Getters) {
  SpelltowerBoard board1({{}});
  EXPECT_EQ(board1.NumRows(), 1);
  EXPECT_EQ(board1.NumCols(), 0);
  EXPECT_EQ(board1.NumStars(), 0);

  SpelltowerBoard board2({{'a', 'B'}});
  EXPECT_EQ(board2.NumRows(), 1);
  EXPECT_EQ(board2.NumCols(), 2);
  EXPECT_EQ(board2.NumStars(), 1);

  SpelltowerBoard board3({{'A'}, {'B'}});
  EXPECT_EQ(board3.NumRows(), 2);
  EXPECT_EQ(board3.NumCols(), 1);
  EXPECT_EQ(board3.NumStars(), 2);
}

TEST(SpelltowerBoardTest, ScoreOfEmptyPathIsZero) {
  SpelltowerBoard board({{'h', 'h', 'h', 'h', 'h'},
                         {'a', 'e', 'i', 'o', 'u'},
                         {'s', 's', 's', 's', 's'}});
  EXPECT_EQ(board.Score({}), 0);
}

TEST(SpelltowerBoardTest, ScoreAdjacentCellsWhenAppropriate) {
  SpelltowerBoard board({{'*', '*', '*', '*', '*'}, {'a', 'a', 'a', 'a', 'a'}});
  EXPECT_EQ(board.Score({{0, 0}, {0, 1}, {0, 2}, {0, 3}}), 0);
  // 5-long word * 5 points (1 per a)
  EXPECT_EQ(board.Score({{0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}}), 25);
}

} // namespace
} // namespace puzzmo