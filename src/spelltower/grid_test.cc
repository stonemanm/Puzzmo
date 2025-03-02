#include "grid.h"

#include <memory>

#include "absl/status/status_matchers.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/shared/letter_count.h"

namespace puzzmo::spelltower {
namespace {

TEST(GridTest, Constructor) {
  Grid grid({"abcdefghi", "jkLmnopqr", "stuvwxyz*", "*zyxwvuts", "rqpOnmlkj",
             "ihgfedcba", "acegibdfh", "jLnprkmoq", "suwy*tvxz", "jjjjjjjjj",
             "kkkkkkkkk", "lllllllll", "mmmmmmmmm"});
  Point p = {0, 0};
  EXPECT_EQ(grid[p]->letter(), 'm');
}

TEST(GridTest, LetterMap) {
  Grid grid({"abcdefghi", "jkLmnopqr", "stuvwxyz*", "*zyxwvuts", "rqpOnmlkj",
             "ihgfedcba", "acegibdfh", "jLnprkmoq", "suwy*tvxz", "jjjjjjjjj",
             "kkkkkkkkk", "lllllllll", "mmmmmmmmm"});
  EXPECT_THAT(
      grid.letter_map()['a'],
      testing::UnorderedElementsAre(grid[{12, 0}], grid[{6, 0}], grid[{7, 8}]));
  EXPECT_THAT(grid.letter_map()['m'],
              testing::UnorderedElementsAre(
                  grid[{0, 0}], grid[{0, 1}], grid[{0, 2}], grid[{0, 3}],
                  grid[{0, 4}], grid[{0, 5}], grid[{0, 6}], grid[{0, 7}],
                  grid[{0, 8}], grid[{5, 6}], grid[{8, 5}], grid[{11, 3}]));
}

TEST(GridTest, ColumnLetterCounts) {
  Grid grid({"abcdefghi", "abcdefghi", "abcdefgh*", "Abcdefghi", "abcdefghi",
             "abcdefghi", "abcdefghi", "abcdefghi", "abcdefghi", "jjjjjjjjj",
             "kkkkkkkkk", "lllllllll", "mmmmmmmmm"});
  EXPECT_THAT(grid.column_letter_counts(),
              testing::ElementsAre(
                  LetterCount("aaaaaaaaajklm"), LetterCount("bbbbbbbbbjlkm"),
                  LetterCount("cccccccccjlkm"), LetterCount("dddddddddjlkm"),
                  LetterCount("eeeeeeeeejlkm"), LetterCount("fffffffffjlkm"),
                  LetterCount("gggggggggjlkm"), LetterCount("hhhhhhhhhjlkm"),
                  LetterCount("iiiiiiiijlkm")));
}

TEST(GridTest, ScoreBonuses) {
  Grid neither({"         ", "         ", "         ", "         ", "         ",
                "         ", "         ", "         ", "         ", "         ",
                "  x      ", "  x      ", "  x      "});
  EXPECT_FALSE(neither.AlmostThere());
  EXPECT_FALSE(neither.FullClear());
  EXPECT_EQ(neither.ScoreBonuses(), 0);

  Grid almost_there({"         ", "         ", "         ", "         ",
                     "         ", "         ", "         ", "         ",
                     "         ", "         ", "         ", "xxxxxxxxx",
                     "xxxxxxxxx"});
  EXPECT_TRUE(almost_there.AlmostThere());
  EXPECT_FALSE(almost_there.FullClear());
  EXPECT_EQ(almost_there.ScoreBonuses(), 1000);

  Grid full_clear({"         ", "         ", "         ", "         ",
                   "         ", "         ", "         ", "         ",
                   "         ", "         ", "         ", "         ",
                   "         "});
  EXPECT_TRUE(full_clear.AlmostThere());
  EXPECT_TRUE(full_clear.FullClear());
  EXPECT_EQ(full_clear.ScoreBonuses(), 2000);
}

TEST(GridTest, AccessibleTilesFrom) {
  Grid grid({"xxxxxxx x", "xxxxxxxxx", "xxxxxxxxx", "xxxxxxxxx", "xxxxxxxxx",
             "xxxxxxxxx", "xxxxxxxxx", "xxxxxxxxx", "xxxxxxxxx", "xxxxxxxxx",
             "xxxxxxxxx", "xxxxxxxxx", "xxxxxxxxx"});
  EXPECT_THAT(grid.AccessibleTilesFrom(grid[{0, 0}]), testing::SizeIs(3));
  EXPECT_THAT(grid.AccessibleTilesFrom(grid[{1, 0}]), testing::SizeIs(5));
  EXPECT_THAT(grid.AccessibleTilesFrom(grid[{1, 1}]), testing::SizeIs(8));
  EXPECT_THAT(grid.AccessibleTilesFrom(grid[{12, 8}]), testing::SizeIs(2));
}

TEST(GridTest, IsPointInRange) {
  Grid grid({"         ", "         ", "         ", "         ", "        x",
             "       xx", "      xxx", "     xxxx", "    xxxxx", "   xxxxxx",
             "  xxxxxxx", " xxxxxxxx", "xxxxxxxxx"});
  EXPECT_THAT(grid.IsPointInRange({0, -1}), testing::IsFalse());
  EXPECT_THAT(grid.IsPointInRange({0, 9}), testing::IsFalse());
  EXPECT_THAT(grid.IsPointInRange({-1, 8}), testing::IsFalse());
  EXPECT_THAT(grid.IsPointInRange({8, 8}), testing::IsTrue());
  EXPECT_THAT(grid.IsPointInRange({9, 8}), testing::IsFalse());
}

TEST(GridTest, PossibleNextTilesForPath) {
  Grid grid({"         ", "         ", "         ", "         ", "        x",
             "       xx", "      xx*", "     xxxx", "    xxxxx", "   xxxxxx",
             "  xxxxxxx", " xxxxxxxx", "xxxxxxxxx"});
  Path path({grid[{6, 7}], grid[{7, 7}], grid[{7, 8}]});
  EXPECT_THAT(grid.PossibleNextTilesForPath(path),
              testing::UnorderedElementsAre(grid[{8, 8}]));
}

TEST(GridTest, TilesAffectedBy) {
  Grid grid({"xxxxxxx x", "xxxxxxxxx", "xxxxxxxxx", "xxxxxxxxx", "xxxxxxxxx",
             "xxxxxxxxx", "xxxxxxxxx", "xxxxxxxxx", "xxxxxxxxx", "xxxxxxxxx",
             "xxxxxxxxx", "xxxxxxxxx", "xxxxxxxxx"});
  EXPECT_THAT(grid.TilesAffectedBy(grid[{0, 0}]),
              testing::UnorderedElementsAre(grid[{0, 1}], grid[{1, 0}]));
  EXPECT_THAT(grid.TilesAffectedBy(grid[{1, 1}]),
              testing::UnorderedElementsAre(grid[{0, 1}], grid[{1, 0}],
                                            grid[{2, 1}], grid[{1, 2}]));
  EXPECT_THAT(grid.TilesAffectedBy(grid[{12, 8}]),
              testing::UnorderedElementsAre(grid[{11, 8}]));
}

TEST(GridTest, TilesRemovedBy) {
  Grid grid({"nnnnnnn n", "mmmmmmm m", "lllllll l", "kkkkkkk k", "iiiijii i",
             "hhhhhhhhh", "ggggggggg", "fffffffff", "eeeeeeeee", "ddddddddd",
             "ccccccccc", "bbbbbbbbb", "aaaaaaaaa"});

  Path short_path({grid[{0, 0}], grid[{0, 1}], grid[{1, 2}]});
  EXPECT_THAT(
      grid.TilesRemovedBy(short_path),
      testing::UnorderedElementsAre(grid[{0, 0}], grid[{0, 1}], grid[{1, 2}]));

  Path path_with_rare_tile({grid[{8, 4}], grid[{7, 4}], grid[{6, 4}]});
  EXPECT_THAT(grid.TilesRemovedBy(path_with_rare_tile),
              testing::UnorderedElementsAre(
                  grid[{8, 4}], grid[{7, 4}], grid[{6, 4}], grid[{8, 0}],
                  grid[{8, 1}], grid[{8, 2}], grid[{8, 3}], grid[{8, 5}],
                  grid[{8, 6}], grid[{8, 8}]));

  Path long_path({grid[{8, 5}], grid[{9, 6}], grid[{10, 6}], grid[{11, 6}],
                  grid[{12, 6}]});
  EXPECT_THAT(grid.TilesRemovedBy(long_path),
              testing::UnorderedElementsAre(
                  grid[{8, 5}], grid[{8, 4}], grid[{8, 6}], grid[{7, 5}],
                  grid[{9, 5}], grid[{9, 6}], grid[{10, 6}], grid[{10, 5}],
                  grid[{11, 6}], grid[{11, 5}], grid[{12, 6}], grid[{12, 5}]));
}

TEST(GridTest, ClearPath) {
  Grid grid({"nnnnnnn n", "mmmmmmm m", "lllllll l", "kkkkkkk k", "iiiijii i",
             "hhhhhhhhh", "ggggggggg", "fffffffff", "eeeeeeeee", "ddddddddd",
             "ccccccccc", "bbbbbbbbb", "aaaaaaaaa"});

  Path short_path({grid[{0, 0}], grid[{0, 1}], grid[{1, 2}]});
  EXPECT_THAT(grid.ClearPath(short_path), absl_testing::IsOk());
  EXPECT_EQ((grid[{0, 1}]->letter()), ('b'));
  EXPECT_EQ((grid[{0, 2}]->letter()), ('a'));
  EXPECT_EQ((grid[{1, 1}]->letter()), ('c'));
  EXPECT_EQ((grid[{1, 2}]->letter()), ('c'));
}

TEST(GridTest, ClearTile) {
  Grid grid({"nnnnnnn n", "mmmmmmm m", "lllllll l", "kkkkkkk k", "iiiijii i",
             "hhhhhhhhh", "ggggggggg", "fffffffff", "eeeeeeeee", "ddddddddd",
             "ccccccccc", "bbbbbbbbb", "aaaaaaaaa"});
  EXPECT_THAT(grid[4], testing::SizeIs(13));
  EXPECT_THAT(grid.ClearTile(grid[{8, 4}]), absl_testing::IsOk());
  EXPECT_EQ((grid[{8, 4}]->letter()), ('k'));
  EXPECT_THAT(grid[4], testing::SizeIs(13));
}

TEST(GridTest, ScorePath) {
  Grid grid({"nnnnnNn n", "mmmmmmm m", "lllllll l", "kkkkkkk k", "Iiiijii i",
             "hhhhhhhhh", "ggggggggg", "fffffffff", "eeeeeeeee", "ddddddddd",
             "ccccccccc", "bbBbbbbbb", "aaaaaaaaa"});

  // a+a+b (6) * length (3) * stars + 1 (2) = 36
  Path short_path({grid[{0, 0}], grid[{0, 1}], grid[{1, 2}]});
  EXPECT_EQ(grid.ScorePath(short_path), 36);

  // j+h+g + i+i+i+i+i+i+i (24) * length (3) * stars + 1 (1) = 72
  Path path_with_rare_tile({grid[{8, 4}], grid[{7, 4}], grid[{6, 4}]});
  EXPECT_EQ(grid.ScorePath(path_with_rare_tile), 72);

  // i+k+l+m+n + j+h+i+k+n+m+l (44) * length (5) * stars + 1 (1) = 220
  Path long_path({grid[{8, 5}], grid[{9, 6}], grid[{10, 6}], grid[{11, 6}],
                  grid[{12, 6}]});
  EXPECT_EQ(grid.ScorePath(long_path), 220);
}

}  // namespace
}  // namespace puzzmo::spelltower