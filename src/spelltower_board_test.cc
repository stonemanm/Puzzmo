#include "spelltower_board.h"

#include <vector>

#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
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
  EXPECT_EQ(board.At({0, 0}), '*');
  EXPECT_FALSE(board.MightHaveWord(""));
}

TEST(SpelltowerBoardTest, At) {
  SpelltowerBoard board({{'a', 'b', 'c'}, {'d', 'e', 'F'}});
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

TEST(SpelltowerBoardTest, MightHaveWord) {
  SpelltowerBoard board({{'a', 'b', 'c', 'd', 'e'},
                         {'f', 'g', 'h', 'i', 'j'},
                         {'a', 'k', 'l', 'm', 'n'}});
  EXPECT_TRUE(board.MightHaveWord(""));
  EXPECT_TRUE(board.MightHaveWord("i"));
  EXPECT_TRUE(board.MightHaveWord("ab"));
  EXPECT_FALSE(board.MightHaveWord("elf"));

  // Note: this is not actually possible for reasons outside the scope of what
  // is checked. Nevertheless, this should return true.
  EXPECT_TRUE(board.MightHaveWord("fig"));
}

TEST(SpelltowerBoardTest, MightHaveWords) {
  SpelltowerBoard board({{'h', 'h', 'h', 'h', 'h'},
                         {'a', 'e', 'i', 'o', 'u'},
                         {'s', 's', 's', 's', 's'}});
  std::vector<std::string> words = {"has", "hash", "his", "hose", "house"};
  // Note: "house" is not actually possible but should nevertheless return true.
  std::vector<std::string> filtered_words = {"has", "his", "hose", "house"};
  EXPECT_EQ(board.MightHaveWords(words), filtered_words);
}

TEST(SpelltowerBoardTest, ValidVonNeumannNeighbors) {
  SpelltowerBoard board({{'h', 'h', 'h', 'h', 'h'},
                         {'a', 'e', 'i', 'o', 'u'},
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
                         {'a', 'e', 'i', 'o', 'u'},
                         {'s', 's', 's', 's', 's'}});

  absl::flat_hash_set<Point> valid_neighbors = {{1, 3}, {0, 3}, {0, 2}, {0, 1},
                                                {1, 1}, {2, 1}, {2, 2}, {2, 3}};
  EXPECT_EQ(board.ValidMooreNeighbors({1, 2}), valid_neighbors);

  valid_neighbors = {{0, 1}, {1, 0}, {1, 1}};
  EXPECT_EQ(board.ValidMooreNeighbors({0, 0}), valid_neighbors);

  valid_neighbors = {{2, 2}, {1, 2}, {1, 1}, {1, 0}, {2, 0}};
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
  EXPECT_THAT(board4.GetAllStarRegexes(),
              testing::UnorderedElementsAre(
                  "a.{2,}b.{1,}c.*d", "a.{2,}b.{2,}d.*c", "a.{4,}c.{1,}b.{2,}d",
                  "a.{4,}c.*d.{2,}b", "a.{5,}d.{2,}b.{1,}c", "a.{5,}d.*c.{1,}b",
                  "b.{2,}a.{4,}c.*d", "b.{2,}a.{5,}d.*c", "b.{1,}c.{4,}a.{5,}d",
                  "b.{1,}c.*d.{5,}a", "b.{2,}d.{5,}a.{4,}c", "b.{2,}d.*c.{4,}a",
                  "c.{1,}b.{2,}a.{5,}d", "c.{1,}b.{2,}d.{5,}a",
                  "c.{4,}a.{2,}b.{2,}d", "c.{4,}a.{5,}d.{2,}b",
                  "c.*d.{2,}b.{2,}a", "c.*d.{5,}a.{2,}b", "d.{2,}b.{1,}c.{4,}a",
                  "d.{2,}b.{2,}a.{4,}c", "d.*c.{1,}b.{2,}a", "d.*c.{4,}a.{2,}b",
                  "d.{5,}a.{2,}b.{1,}c", "d.{5,}a.{4,}c.{1,}b"));
}

} // namespace
} // namespace puzzmo