#include "grid.h"

#include <memory>

#include "absl/status/status_matchers.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/shared/letter_count.h"

namespace puzzmo::spelltower {
namespace {

TEST(GridTest, Constructor) {
  Grid small_grid({"abc", "b*ac", "cabby"});
  EXPECT_EQ((small_grid[{0, 0}]->letter()), 'c');
  EXPECT_EQ((small_grid[{0, 4}]->letter()), 'y');
  EXPECT_EQ((small_grid[{1, 4}]), nullptr);
  EXPECT_EQ((small_grid[{0, 5}]), nullptr);
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
  Path path;
  ASSERT_THAT(path.push_back({grid[{6, 7}], grid[{7, 7}], grid[{7, 8}]}),
              absl_testing::IsOk());
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

  Path short_path;
  ASSERT_THAT(short_path.push_back({grid[{0, 0}], grid[{0, 1}], grid[{1, 2}]}),
              absl_testing::IsOk());
  EXPECT_THAT(
      grid.TilesRemovedBy(short_path),
      testing::UnorderedElementsAre(grid[{0, 0}], grid[{0, 1}], grid[{1, 2}]));

  Path path_with_rare_tile;
  ASSERT_THAT(
      path_with_rare_tile.push_back({grid[{8, 4}], grid[{7, 4}], grid[{6, 4}]}),
      absl_testing::IsOk());
  EXPECT_THAT(grid.TilesRemovedBy(path_with_rare_tile),
              testing::UnorderedElementsAre(
                  grid[{8, 4}], grid[{7, 4}], grid[{6, 4}], grid[{8, 0}],
                  grid[{8, 1}], grid[{8, 2}], grid[{8, 3}], grid[{8, 5}],
                  grid[{8, 6}], grid[{8, 8}]));

  Path long_path;
  ASSERT_THAT(long_path.push_back({grid[{8, 5}], grid[{9, 6}], grid[{10, 6}],
                                   grid[{11, 6}], grid[{12, 6}]}),
              absl_testing::IsOk());
  EXPECT_THAT(grid.TilesRemovedBy(long_path),
              testing::UnorderedElementsAre(
                  grid[{8, 5}], grid[{8, 4}], grid[{8, 6}], grid[{7, 5}],
                  grid[{9, 5}], grid[{9, 6}], grid[{10, 6}], grid[{10, 5}],
                  grid[{11, 6}], grid[{11, 5}], grid[{12, 6}], grid[{12, 5}]));
}

TEST(GridTest, ClearPath) {
  Grid grid({"nnnnnnn n", "mmmmmmm m", "lllllll l", "kkkkkkk k", "iiiijii i",
             "hhhhhhhhh", "ggggggggg", "fffffffff", "eeeeeeeee", "ddddddddd",
             "ccccccccc", "b*bbbbbbb", "aaaaaaaaa"});

  Path long_path;
  ASSERT_THAT(long_path.push_back({grid[{8, 5}], grid[{9, 6}], grid[{10, 6}],
                                   grid[{11, 6}], grid[{12, 6}]}),
              absl_testing::IsOk());
  EXPECT_THAT(grid.ClearPath(long_path), absl_testing::IsOk());

  Path short_path;
  ASSERT_THAT(short_path.push_back({grid[{0, 0}], grid[{0, 1}], grid[{1, 2}]}),
              absl_testing::IsOk());
  EXPECT_THAT(grid.ClearPath(short_path), absl_testing::IsOk());
  EXPECT_EQ((grid[{0, 1}]->letter()), ('c'));
  EXPECT_EQ((grid[{0, 2}]->letter()), ('a'));
  EXPECT_EQ((grid[{1, 1}]->letter()), ('d'));
  EXPECT_EQ((grid[{1, 2}]->letter()), ('c'));
}

TEST(GridTest, ScorePath) {
  Grid grid({"nnnnnNn n", "mmmmmmm m", "lllllll l", "kkkkkkk k", "Iiiijii i",
             "hhhhhhhhh", "ggggggggg", "fffffffff", "eeeeeeeee", "ddddddddd",
             "ccccccccc", "bbBbbbbbb", "aaaaaaaaa"});

  // a+a+b (6) * length (3) * stars + 1 (2) = 36
  Path short_path;
  ASSERT_THAT(short_path.push_back({grid[{0, 0}], grid[{0, 1}], grid[{1, 2}]}),
              absl_testing::IsOk());
  EXPECT_EQ(grid.ScorePath(short_path), 36);

  // j+h+g + i+i+i+i+i+i+i (24) * length (3) * stars + 1 (1) = 72
  Path path_with_rare_tile;
  ASSERT_THAT(
      path_with_rare_tile.push_back({grid[{8, 4}], grid[{7, 4}], grid[{6, 4}]}),
      absl_testing::IsOk());
  EXPECT_EQ(grid.ScorePath(path_with_rare_tile), 72);

  // i+k+l+m+n + j+h+i+k+n+m+l (44) * length (5) * stars + 1 (1) = 220
  Path long_path;
  ASSERT_THAT(long_path.push_back({grid[{8, 5}], grid[{9, 6}], grid[{10, 6}],
                                   grid[{11, 6}], grid[{12, 6}]}),
              absl_testing::IsOk());
  EXPECT_EQ(grid.ScorePath(long_path), 220);
}

TEST(GridTest, VisualizePath) {
  Grid grid({"   e", "   vi", "  Iatp", " kd.dcHc", "enkolgscr", "ssrsaamfq"});
  Path path;
  ASSERT_THAT(
      path.push_back({grid[{2, 2}], grid[{3, 2}], grid[{4, 3}], grid[{3, 3}]}),
      absl_testing::IsOk());
  ASSERT_EQ(path.word(), "diva");
  EXPECT_EQ(absl::StrFormat("%v", path), "d (2,2), I (3,2), v (4,3), a (3,3)");
  EXPECT_EQ(grid.VisualizePath(path),
            absl::StrJoin({"   .", "   v.", "  Ia..", " .d+....", ".........",
                           "........."},
                          "\n"));
}

TEST(GridTest, AllStarRegex) {
  Grid grid({"AxxxDE"});
  EXPECT_THAT(
      grid.AllStarRegex(),
      testing::StrEq(
          "(.*a.{3,}d.{0,}e.*)|(.*a.{4,}e.{0,}d.*)|(.*d.{3,}a.{4,}e.*)|(.*d.{0,"
          "}e.{4,}a.*)|(.*e.{4,}a.{3,}d.*)|(.*e.{0,}d.{3,}a.*)"));
}

TEST(GridTest, AbslStringify) {
  const std::vector<std::string> grid_string = {
      "   e", "   vi", "  iatp", " kd.dcHc", "enkolgscr", "ssrsaamfq"};
  Grid grid(grid_string);
  EXPECT_EQ(absl::StrFormat("%v", grid), absl::StrJoin(grid_string, "\n"));
}

}  // namespace
}  // namespace puzzmo::spelltower