#include "grid.h"

#include <memory>

#include "absl/status/status.h"
#include "absl/status/status_matchers.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/shared/letter_count.h"

namespace puzzmo::spelltower {
namespace {

using absl_testing::IsOk;
using absl_testing::StatusIs;
using testing::ElementsAre;
using testing::IsFalse;
using testing::IsTrue;
using testing::SizeIs;
using testing::StrEq;
using testing::UnorderedElementsAre;

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
  EXPECT_THAT(grid.letter_map()['a'],
              UnorderedElementsAre(grid[{12, 0}], grid[{6, 0}], grid[{7, 8}]));
  EXPECT_THAT(grid.letter_map()['m'],
              UnorderedElementsAre(grid[{0, 0}], grid[{0, 1}], grid[{0, 2}],
                                   grid[{0, 3}], grid[{0, 4}], grid[{0, 5}],
                                   grid[{0, 6}], grid[{0, 7}], grid[{0, 8}],
                                   grid[{5, 6}], grid[{8, 5}], grid[{11, 3}]));
}

TEST(GridTest, ColumnLetterCounts) {
  Grid grid({"abcdefghi", "abcdefghi", "abcdefgh*", "Abcdefghi", "abcdefghi",
             "abcdefghi", "abcdefghi", "abcdefghi", "abcdefghi", "jjjjjjjjj",
             "kkkkkkkkk", "lllllllll", "mmmmmmmmm"});
  EXPECT_THAT(
      grid.column_letter_counts(),
      ElementsAre(LetterCount("aaaaaaaaajklm"), LetterCount("bbbbbbbbbjlkm"),
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
  EXPECT_THAT(grid.AccessibleTilesFrom(grid[{0, 0}]), SizeIs(3));
  EXPECT_THAT(grid.AccessibleTilesFrom(grid[{1, 0}]), SizeIs(5));
  EXPECT_THAT(grid.AccessibleTilesFrom(grid[{1, 1}]), SizeIs(8));
  EXPECT_THAT(grid.AccessibleTilesFrom(grid[{12, 8}]), SizeIs(2));
}

TEST(GridTest, IsPointInRange) {
  Grid grid({"         ", "         ", "         ", "         ", "        x",
             "       xx", "      xxx", "     xxxx", "    xxxxx", "   xxxxxx",
             "  xxxxxxx", " xxxxxxxx", "xxxxxxxxx"});
  EXPECT_THAT(grid.IsPointInRange({0, -1}), IsFalse());
  EXPECT_THAT(grid.IsPointInRange({0, 9}), IsFalse());
  EXPECT_THAT(grid.IsPointInRange({-1, 8}), IsFalse());
  EXPECT_THAT(grid.IsPointInRange({8, 8}), IsTrue());
  EXPECT_THAT(grid.IsPointInRange({9, 8}), IsFalse());
}

TEST(GridTest, PossibleNextTilesForPath) {
  Grid grid({"         ", "         ", "         ", "         ", "        x",
             "       xx", "      xx*", "     xxxx", "    xxxxx", "   xxxxxx",
             "  xxxxxxx", " xxxxxxxx", "xxxxxxxxx"});
  Path path;
  ASSERT_THAT(path.push_back({grid[{6, 7}], grid[{7, 7}], grid[{7, 8}]}),
              IsOk());
  EXPECT_THAT(grid.PossibleNextTilesForPath(path),
              UnorderedElementsAre(grid[{8, 8}]));
}

TEST(GridTest, TilesRemovedBy) {
  Grid grid({"nnnnnnn n", "mmmmmmm m", "lllllll l", "kkkkkkk k", "iiiijii i",
             "hhhhhhhhh", "ggggggggg", "fffffffff", "eeeeeeeee", "ddddddddd",
             "ccccccccc", "bbbbbbbbb", "aaaaaaaaa"});

  Path short_path;
  ASSERT_THAT(short_path.push_back({grid[{0, 0}], grid[{0, 1}], grid[{1, 2}]}),
              IsOk());
  EXPECT_THAT(grid.TilesRemovedBy(short_path),
              ElementsAre(grid[{0, 0}], grid[{0, 1}], grid[{1, 2}]));

  Path path_with_rare_tile;
  ASSERT_THAT(
      path_with_rare_tile.push_back({grid[{6, 4}], grid[{7, 4}], grid[{8, 4}]}),
      IsOk());
  EXPECT_THAT(
      grid.TilesRemovedBy(path_with_rare_tile),
      ElementsAre(grid[{8, 0}], grid[{8, 1}], grid[{8, 2}], grid[{8, 3}],
                  grid[{8, 4}], grid[{7, 4}], grid[{6, 4}], grid[{8, 5}],
                  grid[{8, 6}], grid[{8, 8}]));

  Path long_path;
  ASSERT_THAT(long_path.push_back({grid[{8, 5}], grid[{9, 6}], grid[{10, 6}],
                                   grid[{11, 6}], grid[{12, 6}]}),
              IsOk());
  EXPECT_THAT(
      grid.TilesRemovedBy(long_path),
      ElementsAre(grid[{8, 4}], grid[{12, 5}], grid[{11, 5}], grid[{10, 5}],
                  grid[{9, 5}], grid[{8, 5}], grid[{7, 5}], grid[{12, 6}],
                  grid[{11, 6}], grid[{10, 6}], grid[{9, 6}], grid[{8, 6}]));
}

TEST(GridTest, ClearPath) {
  Grid grid({"nnnnnnn n", "mmmmmmm m", "lllllll l", "kkkkkkk k", "iiiijii i",
             "hhhhhhhhh", "ggggggggg", "fffffffff", "eeeeeeeee", "ddddddddd",
             "ccccccccc", "b*bbbbbbb", "aaaaaaaaa"});
  std::shared_ptr<Tile> tile_8_5 = grid[{8, 5}];

  Path long_path;
  ASSERT_THAT(long_path.push_back({tile_8_5, grid[{9, 6}], grid[{10, 6}],
                                   grid[{11, 6}], grid[{12, 6}]}),
              IsOk());
  EXPECT_THAT(grid.ClearPath(long_path), IsOk());
  EXPECT_THAT(tile_8_5->is_on_grid(), IsFalse());

  Path short_path;
  ASSERT_THAT(short_path.push_back({grid[{0, 0}], grid[{0, 1}], grid[{1, 2}]}),
              IsOk());
  EXPECT_THAT(grid.ClearPath(short_path), IsOk());
  EXPECT_EQ((grid[{0, 1}]->letter()), ('c'));
  EXPECT_EQ((grid[{0, 2}]->letter()), ('a'));
  EXPECT_EQ((grid[{1, 1}]->letter()), ('d'));
  EXPECT_EQ((grid[{1, 2}]->letter()), ('c'));
}

TEST(GridTest, RevertPathAndReset) {
  Grid grid({"nnnnnnn n", "mmmmmmm m", "lllllll l", "kkkkkkk k", "iiiijii i",
             "hhhhhhhhh", "ggggggggg", "fffffffff", "eeeeeeeee", "ddddddddd",
             "ccccccccc", "b*bbbbbbb", "aaaaaaaaa"});
  std::shared_ptr<Tile> tile_8_5 = grid[{8, 5}];
  EXPECT_THAT(grid.RevertLastClear(),
              StatusIs(absl::StatusCode::kFailedPrecondition));

  std::vector<std::vector<std::shared_ptr<Tile>>> starting_tiles = grid.tiles();
  Path long_path;
  ASSERT_THAT(long_path.push_back({tile_8_5, grid[{9, 6}], grid[{10, 6}],
                                   grid[{11, 6}], grid[{12, 6}]}),
              IsOk());
  ASSERT_THAT(grid.ClearPath(long_path), IsOk());
  ASSERT_THAT(tile_8_5->is_on_grid(), IsFalse());

  std::vector<std::vector<std::shared_ptr<Tile>>> midway_tiles = grid.tiles();
  Path short_path;
  ASSERT_THAT(short_path.push_back({grid[{0, 0}], grid[{0, 1}], grid[{1, 2}]}),
              IsOk());
  ASSERT_THAT(grid.ClearPath(short_path), IsOk());

  EXPECT_THAT(grid.RevertLastClear(), IsOk());
  EXPECT_EQ(grid.tiles(), midway_tiles);

  EXPECT_THAT(grid.RevertLastClear(), IsOk());
  EXPECT_EQ(grid.tiles(), starting_tiles);
  EXPECT_THAT(tile_8_5->is_on_grid(), IsTrue());

  EXPECT_THAT(grid.RevertLastClear(),
              StatusIs(absl::StatusCode::kFailedPrecondition));

  ASSERT_THAT(grid.ClearPath(long_path), IsOk());
  EXPECT_THAT(grid.reset(), IsOk());
  EXPECT_EQ(grid.tiles(), starting_tiles);

  ASSERT_THAT(grid.ClearPath(long_path), IsOk());
  ASSERT_THAT(grid.ClearPath(short_path), IsOk());
  EXPECT_THAT(grid.reset(), IsOk());
  EXPECT_EQ(grid.tiles(), starting_tiles);
}

TEST(GridTest, TilesBeneathPath) {
  Grid grid({"aaa", "bbb", "ccc", "ddd"});
  Path path;
  ASSERT_THAT(path.push_back({grid[{1, 0}], grid[{2, 1}], grid[{3, 0}]}),
              IsOk());
  EXPECT_THAT(grid.TilesBeneathPath(path),
              UnorderedElementsAre(grid[{0, 0}], grid[{2, 0}], grid[{0, 1}],
                                   grid[{1, 1}]));
}

TEST(GridTest, TilesBeneathEachPathTile) {
  Grid grid({"aaa", "bbb", "ccc", "ddd"});
  Path path;
  ASSERT_THAT(path.push_back({grid[{1, 0}], grid[{2, 1}], grid[{3, 0}]}),
              IsOk());
  EXPECT_THAT(grid.TilesBeneathEachPathTile(path),
              ElementsAre(ElementsAre(grid[{0, 0}]),
                          ElementsAre(grid[{0, 1}], grid[{1, 1}]),
                          ElementsAre(grid[{0, 0}], grid[{2, 0}])));
}

TEST(GridTest, ScorePath) {
  Grid grid({"nnnnnNn n", "mmmmmmm m", "lllllll l", "kkkkkkk k", "Iiiijii i",
             "hhhhhhhhh", "ggggggggg", "fffffffff", "eeeeeeeee", "ddddddddd",
             "ccccccccc", "bbBbbbbbb", "aaaaaaaaa"});

  // a+a+b (6) * length (3) * stars + 1 (2) = 36
  Path short_path;
  ASSERT_THAT(short_path.push_back({grid[{0, 0}], grid[{0, 1}], grid[{1, 2}]}),
              IsOk());
  EXPECT_EQ(grid.ScorePath(short_path), 36);

  // j+h+g + i+i+i+i+i+i+i (24) * length (3) * stars + 1 (1) = 72
  Path path_with_rare_tile;
  ASSERT_THAT(
      path_with_rare_tile.push_back({grid[{8, 4}], grid[{7, 4}], grid[{6, 4}]}),
      IsOk());
  EXPECT_EQ(grid.ScorePath(path_with_rare_tile), 72);

  // i+k+l+m+n + j+h+i+k+n+m+l (44) * length (5) * stars + 1 (1) = 220
  Path long_path;
  ASSERT_THAT(long_path.push_back({grid[{8, 5}], grid[{9, 6}], grid[{10, 6}],
                                   grid[{11, 6}], grid[{12, 6}]}),
              IsOk());
  EXPECT_EQ(grid.ScorePath(long_path), 220);
}

TEST(GridTest, VisualizePath) {
  Grid grid({"   e", "   vi", "  Iatp", " kd.dcHc", "enkolgscr", "ssrsaamfq"});
  Path path;
  ASSERT_THAT(
      path.push_back({grid[{2, 2}], grid[{3, 2}], grid[{4, 3}], grid[{3, 3}]}),
      IsOk());
  ASSERT_EQ(path.word(), "diva");
  EXPECT_EQ(absl::StrFormat("%v", path), "d (2,2), I (3,2), v (4,3), a (3,3)");
  EXPECT_EQ(grid.VisualizePath(path),
            absl::StrJoin({"   .", "   v.", "  Ia..", " .d+....", ".........",
                           "........."},
                          "\n"));
}

TEST(GridTest, NStarRegex) {
  Grid grid({"AxxxDE"});
  EXPECT_THAT(
      grid.NStarRegex(3),
      StrEq(
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