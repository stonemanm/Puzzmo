#include "spelltower_board.h"

#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace puzzmo {
namespace {

TEST(SpelltowerBoardTest, EmptyConstructor) {
  SpelltowerBoard board({});
  EXPECT_EQ(board.NumRows(), 0);
  EXPECT_EQ(board.NumCols(), 0);
  EXPECT_FALSE(board.HasPoint({0, 0}));
  EXPECT_FALSE(board.HasPoint(0, 0));
  EXPECT_EQ(board.char_at({0, 0}), '*');
}

TEST(SpelltowerBoardTest, char_at) {
  SpelltowerBoard board({{'a', 'b', 'c'}, {'d', 'e', 'F'}});
  EXPECT_EQ(board.char_at({0, 0}), 'a');
  EXPECT_EQ(board.char_at(0, 0), 'a');
  EXPECT_EQ(board.char_at({1, 2}), 'f');
  EXPECT_EQ(board.char_at(1, 2), 'f');
  EXPECT_EQ(board.char_at({-1, 0}), '*');
  EXPECT_EQ(board.char_at(-1, 0), '*');
  EXPECT_EQ(board.char_at({0, -1}), '*');
  EXPECT_EQ(board.char_at(0, -1), '*');
  EXPECT_EQ(board.char_at({2, 0}), '*');
  EXPECT_EQ(board.char_at(2, 0), '*');
  EXPECT_EQ(board.char_at({0, 3}), '*');
  EXPECT_EQ(board.char_at(0, 3), '*');
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
  EXPECT_EQ(board.ScorePath({}), 0);
}

TEST(SpelltowerBoardTest, ScoreAdjacentCellsWhenAppropriate) {
  SpelltowerBoard board({{'*', '*', '*', '*', '*'}, {'a', 'a', 'a', 'a', 'a'}});
  EXPECT_EQ(board.ScorePath(SpelltowerPath({{0, 0}, {0, 1}, {0, 2}, {0, 3}})),
            0);
  // 5-long word * 5 points (1 per a)
  EXPECT_EQ(
      board.ScorePath(SpelltowerPath({{0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}})),
      25);
}

TEST(SpelltowerBoardTest, ValidVonNeumannNeighbors) {
  SpelltowerBoard board({{'h', 'h', 'h', 'h', 'h'},
                         {'a', '*', 'i', 'o', 'u'},
                         {'s', 's', 's', 's', 's'}});

  absl::flat_hash_set<Point> valid_neighbors = {{1, 3}, {0, 2}, {1, 1}, {2, 2}};
  EXPECT_EQ(board.ValidVonNeumannNeighbors({1, 2}), valid_neighbors);

  valid_neighbors = {{0, 1}, {1, 0}};
  EXPECT_EQ(board.ValidVonNeumannNeighbors({0, 0}), valid_neighbors);

  valid_neighbors = {{2, 2}, {1, 1}, {2, 0}};
  EXPECT_EQ(board.ValidVonNeumannNeighbors({2, 1}), valid_neighbors);

  valid_neighbors = {{0, 0}};
  EXPECT_EQ(board.ValidVonNeumannNeighbors({-1, 0}), valid_neighbors);
}

TEST(SpelltowerBoardTest, ValidMooreNeighbors) {
  SpelltowerBoard board({{'h', 'h', 'h', 'h', 'h'},
                         {'a', '*', 'i', 'o', 'u'},
                         {'s', 's', 's', 's', 's'}});

  absl::flat_hash_set<Point> valid_neighbors = {{1, 3}, {0, 3}, {0, 2}, {0, 1},
                                                {2, 1}, {2, 2}, {2, 3}};
  EXPECT_EQ(board.ValidMooreNeighbors({1, 2}), valid_neighbors);

  valid_neighbors = {{0, 1}, {1, 0}};
  EXPECT_EQ(board.ValidMooreNeighbors({0, 0}), valid_neighbors);

  valid_neighbors = {{2, 2}, {1, 2}, {1, 0}, {2, 0}};
  EXPECT_EQ(board.ValidMooreNeighbors({2, 1}), valid_neighbors);

  valid_neighbors = {{0, 0}, {0, 1}};
  EXPECT_EQ(board.ValidMooreNeighbors({-1, 0}), valid_neighbors);
}

TEST(SpelltowerBoardTest, GetAllStarRegexes) {
  SpelltowerBoard board0({{'x'}, {'x'}, {'x'}, {'x'}, {'x'}, {'x'}, {'x'}});
  SpelltowerBoard board1({{'A'}, {'x'}, {'x'}, {'x'}, {'x'}, {'x'}, {'x'}});
  SpelltowerBoard board2({{'A'}, {'x'}, {'x'}, {'B'}, {'x'}, {'x'}, {'x'}});
  SpelltowerBoard board3({{'A'}, {'x'}, {'x'}, {'B'}, {'x'}, {'C'}, {'x'}});
  SpelltowerBoard board4({{'A'}, {'x'}, {'x'}, {'B'}, {'x'}, {'C'}, {'D'}});

  EXPECT_THAT(board0.GetAllStarRegexes(), testing::IsEmpty());
  EXPECT_THAT(board1.GetAllStarRegexes(), testing::UnorderedElementsAre("a"));
  EXPECT_THAT(board2.GetAllStarRegexes(),
              testing::UnorderedElementsAre("a.{2,}b", "b.{2,}a"));
  EXPECT_THAT(board3.GetAllStarRegexes(),
              testing::UnorderedElementsAre("a.{2,}b.{1,}c", "a.{4,}c.{1,}b",
                                            "b.{2,}a.{4,}c", "b.{1,}c.{4,}a",
                                            "c.{4,}a.{2,}b", "c.{1,}b.{2,}a"));
  EXPECT_THAT(
      board4.GetAllStarRegexes(),
      testing::UnorderedElementsAre(
          "a.{2,}b.{1,}c.*d", "a.{2,}b.{2,}d.*c", "a.{4,}c.{1,}b.{2,}d",
          "a.{4,}c.*d.{2,}b", "a.{5,}d.{2,}b.{1,}c", "a.{5,}d.*c.{1,}b",
          "b.{2,}a.{4,}c.*d", "b.{2,}a.{5,}d.*c", "b.{1,}c.{4,}a.{5,}d",
          "b.{1,}c.*d.{5,}a", "b.{2,}d.{5,}a.{4,}c", "b.{2,}d.*c.{4,}a",
          "c.{1,}b.{2,}a.{5,}d", "c.{1,}b.{2,}d.{5,}a", "c.{4,}a.{2,}b.{2,}d",
          "c.{4,}a.{5,}d.{2,}b", "c.*d.{2,}b.{2,}a", "c.*d.{5,}a.{2,}b",
          "d.{2,}b.{1,}c.{4,}a", "d.{2,}b.{2,}a.{4,}c", "d.*c.{1,}b.{2,}a",
          "d.*c.{4,}a.{2,}b", "d.{5,}a.{2,}b.{1,}c", "d.{5,}a.{4,}c.{1,}b"));
}

TEST(SpelltowerBoardTest, ClearPoint) {
  SpelltowerBoard board({"monkey", "paneL", "vines", "fluTe", "finds"});

  EXPECT_THAT(board.board()[3], testing::SizeIs(6));
  std::vector<Point> stars_before = {{1, 4}, {3, 3}};
  ASSERT_EQ(board.StarLocations(), stars_before);
  Point t_before = {3, 3};
  ASSERT_THAT(board.letter_map()['t'], testing::ElementsAre(t_before));

  board.ClearPoint({3, 1});

  EXPECT_THAT(board.board()[3], testing::SizeIs(6));
  std::vector<Point> stars_after = {{1, 4}, {3, 2}};
  EXPECT_EQ(board.StarLocations(), stars_after);
  Point t_after = {3, 2};
  EXPECT_THAT(board.letter_map()['t'], testing::ElementsAre(t_after));
}

TEST(SpelltowerBoardTest, ClearPath) {
  SpelltowerBoard board({"monkey", "paneL", "vines", "fluTe", "finds"});
  SpelltowerPath path({{1, 0}, {1, 1}, {1, 2}});  // "pan"

  std::vector<Point> stars_before = {{1, 4}, {3, 3}};
  ASSERT_EQ(board.StarLocations(), stars_before);

  board.ClearPath(path);

  std::vector<Point> stars_after = {{1, 1}, {3, 3}};
  EXPECT_EQ(board.StarLocations(), stars_after);
}

}  // namespace
}  // namespace puzzmo