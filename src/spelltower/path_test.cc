#include "path.h"

#include <memory>

#include "absl/status/status.h"
#include "absl/status/status_matchers.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace puzzmo::spelltower {
namespace {

using absl_testing::IsOk;
using absl_testing::StatusIs;

TEST(PathTest, EmptyConstructor) {
  Path path;
  EXPECT_THAT(path.tiles(), testing::IsEmpty());
  EXPECT_TRUE(path.empty());
  EXPECT_EQ(path.size(), 0);
  EXPECT_THAT(path.row_on_simple_board(), testing::IsEmpty());
  EXPECT_THAT(path.simple_board(), testing::SizeIs(9));
  EXPECT_THAT(path.simple_board()[0], testing::IsEmpty());
  EXPECT_EQ(path.star_count(), 0);
}

TEST(PathTest, PushBackOneTile) {
  Point p0 = {0, 0};
  Point p1 = {1, 1};
  Point p2 = {0, 1};
  std::shared_ptr<Tile> t0 = std::make_shared<Tile>(p0, 'A');
  std::shared_ptr<Tile> t1 = std::make_shared<Tile>(p1, 'B');
  std::shared_ptr<Tile> t2 = std::make_shared<Tile>(p2, 'c');
  Path path;

  EXPECT_THAT(path.push_back(t0), IsOk());
  EXPECT_THAT(path.tiles(), testing::ElementsAre(t0));
  EXPECT_FALSE(path.empty());
  EXPECT_EQ(path.size(), 1);
  EXPECT_THAT(path.row_on_simple_board(), testing::ElementsAre(0));
  EXPECT_THAT(path.simple_board()[0], testing::ElementsAre(0));
  EXPECT_EQ(path.star_count(), 1);

  EXPECT_THAT(path.push_back(t1), IsOk());
  EXPECT_THAT(path.tiles(), testing::ElementsAre(t0, t1));
  EXPECT_FALSE(path.empty());
  EXPECT_EQ(path.size(), 2);
  EXPECT_THAT(path.row_on_simple_board(), testing::ElementsAre(0, 0));
  EXPECT_THAT(path.simple_board()[0], testing::ElementsAre(0));
  EXPECT_THAT(path.simple_board()[1], testing::ElementsAre(1));
  EXPECT_EQ(path.star_count(), 2);

  EXPECT_THAT(path.push_back(t2), IsOk());
  EXPECT_THAT(path.tiles(), testing::ElementsAre(t0, t1, t2));
  EXPECT_FALSE(path.empty());
  EXPECT_EQ(path.size(), 3);
  EXPECT_THAT(path.row_on_simple_board(), testing::ElementsAre(0, 1, 0));
  EXPECT_THAT(path.simple_board()[0], testing::ElementsAre(0));
  EXPECT_THAT(path.simple_board()[1], testing::ElementsAre(2, 1));
  EXPECT_EQ(path.star_count(), 2);
}

TEST(PathTest, PushBackMultipleTiles) {
  std::shared_ptr<Tile> t0 = std::make_shared<Tile>(0, 0, 'A');
  std::shared_ptr<Tile> t1 = std::make_shared<Tile>(1, 1, 'B');
  std::shared_ptr<Tile> t2 = std::make_shared<Tile>(0, 1, 'c');
  Path path;

  EXPECT_THAT(path.push_back({t0, t1, t2}), IsOk());
  EXPECT_THAT(path.tiles(), testing::ElementsAre(t0, t1, t2));
  EXPECT_FALSE(path.empty());
  EXPECT_EQ(path.size(), 3);
  EXPECT_THAT(path.row_on_simple_board(), testing::ElementsAre(0, 1, 0));
  EXPECT_THAT(path.simple_board()[0], testing::ElementsAre(0));
  EXPECT_THAT(path.simple_board()[1], testing::ElementsAre(2, 1));
  EXPECT_EQ(path.star_count(), 2);
}

TEST(PathTest, PushBackNullptrFails) {
  Path path;
  Path unchanged = path;
  EXPECT_THAT(path.push_back(nullptr),
              StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_EQ(path, unchanged);
}

TEST(PathTest, PushBackBlankTileFails) {
  std::shared_ptr<Tile> blank_tile = std::make_shared<Tile>(0, 0);
  Path path;
  Path unchanged = path;
  EXPECT_THAT(path.push_back(blank_tile),
              StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_EQ(path, unchanged);
}

//   1
// 0
TEST(PathTest, PushBackFailsWithColumnGap) {
  Path path;
  ASSERT_THAT(path.push_back(std::make_shared<Tile>(0, 0, 'a')), IsOk());
  Path unchanged = path;
  EXPECT_THAT(path.push_back(std::make_shared<Tile>(1, 2, 'b')),
              StatusIs(absl::StatusCode::kOutOfRange));
  EXPECT_EQ(path, unchanged);
}

TEST(PathTest, PopBack) {
  std::shared_ptr<Tile> t0 = std::make_shared<Tile>(0, 0, 'A');
  std::shared_ptr<Tile> t1 = std::make_shared<Tile>(1, 1, 'B');
  std::shared_ptr<Tile> t2 = std::make_shared<Tile>(0, 1, 'c');
  Path path;
  ASSERT_THAT(path.push_back({t0, t1, t2}), IsOk());

  path.pop_back();
  EXPECT_THAT(path.tiles(), testing::ElementsAre(t0, t1));
  EXPECT_FALSE(path.empty());
  EXPECT_EQ(path.size(), 2);
  EXPECT_THAT(path.row_on_simple_board(), testing::ElementsAre(0, 0));
  EXPECT_THAT(path.simple_board()[0], testing::ElementsAre(0));
  EXPECT_THAT(path.simple_board()[1], testing::ElementsAre(1));
  EXPECT_EQ(path.star_count(), 2);

  path.pop_back();
  EXPECT_THAT(path.tiles(), testing::ElementsAre(t0));
  EXPECT_FALSE(path.empty());
  EXPECT_EQ(path.size(), 1);
  EXPECT_THAT(path.row_on_simple_board(), testing::ElementsAre(0));
  EXPECT_THAT(path.simple_board()[0], testing::ElementsAre(0));
  EXPECT_EQ(path.star_count(), 1);

  path.pop_back();
  EXPECT_THAT(path.tiles(), testing::IsEmpty());
  EXPECT_TRUE(path.empty());
  EXPECT_EQ(path.size(), 0);
  EXPECT_THAT(path.row_on_simple_board(), testing::IsEmpty());
  EXPECT_THAT(path.simple_board()[0], testing::IsEmpty());
  EXPECT_EQ(path.star_count(), 0);
}

TEST(PathTest, IsContinuous) {
  Path path;
  ASSERT_THAT(
      path.push_back(
          {std::make_shared<Tile>(0, 0, 'a'), std::make_shared<Tile>(1, 0, 'b'),
           std::make_shared<Tile>(2, 0, 'b'), std::make_shared<Tile>(3, 0, 'e'),
           std::make_shared<Tile>(5, 1, 'y')}),
      IsOk());
  EXPECT_THAT(path.IsContinuous(), testing::IsFalse());  // row 5
  ASSERT_THAT(path[4]->Drop(1), IsOk());
  EXPECT_THAT(path.IsContinuous(), testing::IsTrue());  // row 4
  ASSERT_THAT(path[4]->Drop(1), IsOk());
  EXPECT_THAT(path.IsContinuous(), testing::IsTrue());  // row 3
  ASSERT_THAT(path[4]->Drop(1), IsOk());
  EXPECT_THAT(path.IsContinuous(), testing::IsTrue());  // row 2
  ASSERT_THAT(path[4]->Drop(1), IsOk());
  EXPECT_THAT(path.IsContinuous(), testing::IsFalse());  // row 1
}

TEST(PathTest, IsPossible) {
  std::shared_ptr<Tile> t00 = std::make_shared<Tile>(0, 0, 'a');
  std::shared_ptr<Tile> t10 = std::make_shared<Tile>(1, 0, 'b');
  std::shared_ptr<Tile> t20 = std::make_shared<Tile>(2, 0, 'c');
  std::shared_ptr<Tile> t30 = std::make_shared<Tile>(3, 0, 'd');
  std::shared_ptr<Tile> t01 = std::make_shared<Tile>(0, 1, 'e');
  std::shared_ptr<Tile> t11 = std::make_shared<Tile>(1, 1, 'f');
  std::shared_ptr<Tile> t21 = std::make_shared<Tile>(2, 1, 'g');
  std::shared_ptr<Tile> t12 = std::make_shared<Tile>(1, 2, 'h');

  Path empty_path;
  EXPECT_THAT(empty_path.IsPossible(), IsOk());

  //   2
  // 01
  Path simple_and_true;
  ASSERT_THAT(simple_and_true.push_back({t00, t01, t12}), IsOk());
  EXPECT_THAT(simple_and_true.IsPossible(), IsOk());

  // 1
  //
  // 0
  Path simple_drop;
  ASSERT_THAT(simple_drop.push_back({t00, t20}), IsOk());
  EXPECT_THAT(simple_drop.IsPossible(), IsOk());

  //  1
  //
  // 0
  Path simple_drop_offset;
  ASSERT_THAT(simple_drop_offset.push_back({t00, t21}), IsOk());
  EXPECT_THAT(simple_drop_offset.IsPossible(), IsOk());

  // 1
  // 2
  // 0
  Path false_bc_interrupted_column;
  ASSERT_THAT(false_bc_interrupted_column.push_back({t00, t20, t10}), IsOk());
  EXPECT_THAT(false_bc_interrupted_column.IsPossible(),
              StatusIs(absl::StatusCode::kInvalidArgument));

  // 3
  // 0
  // 12
  Path false_bc_tiles_beneath;
  ASSERT_THAT(false_bc_tiles_beneath.push_back({t10, t00, t01, t20}), IsOk());
  EXPECT_THAT(false_bc_tiles_beneath.IsPossible(),
              StatusIs(absl::StatusCode::kInvalidArgument));

  // 4
  //  3
  //  52
  // 01
  Path complicated_and_true;
  ASSERT_THAT(complicated_and_true.push_back({t00, t01, t12, t21, t30, t11}),
              IsOk());
  EXPECT_THAT(complicated_and_true.IsPossible(), IsOk());

  // 6
  // 54
  // 073
  // 12
  Path complicated_and_false;
  ASSERT_THAT(
      complicated_and_false.push_back({t10, t00, t01, t12, t21, t20, t30, t11}),
      IsOk());
  EXPECT_THAT(complicated_and_false.IsPossible(),
              StatusIs(absl::StatusCode::kInvalidArgument));
}

TEST(PathTest, AbslStringify) {
  Path path;
  ASSERT_THAT(
      path.push_back(
          {std::make_shared<Tile>(0, 0, 'q'), std::make_shared<Tile>(1, 1, 'r'),
           std::make_shared<Tile>(2, 0, 'S'), std::make_shared<Tile>(2, 1, 'T'),
           std::make_shared<Tile>(3, 2, 'u')}),
      IsOk());
  EXPECT_EQ(absl::StrFormat("%v", path), "qrSTu");
}

}  // namespace
}  // namespace puzzmo::spelltower