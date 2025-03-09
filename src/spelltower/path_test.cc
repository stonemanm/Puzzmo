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
  EXPECT_THAT(path.lowest_legal_row(), testing::IsEmpty());
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
  EXPECT_THAT(path.lowest_legal_row(), testing::ElementsAre(0));
  EXPECT_THAT(path.simple_board()[0], testing::ElementsAre(0));
  EXPECT_THAT(path.adjusted_points(), testing::ElementsAre(p0));
  EXPECT_EQ(path.star_count(), 1);

  EXPECT_THAT(path.push_back(t1), IsOk());
  EXPECT_THAT(path.tiles(), testing::ElementsAre(t0, t1));
  EXPECT_FALSE(path.empty());
  EXPECT_EQ(path.size(), 2);
  EXPECT_THAT(path.lowest_legal_row(), testing::ElementsAre(0, 0));
  EXPECT_THAT(path.simple_board()[0], testing::ElementsAre(0));
  EXPECT_THAT(path.simple_board()[1], testing::ElementsAre(1));
  EXPECT_EQ(path.star_count(), 2);

  EXPECT_THAT(path.push_back(t2), IsOk());
  EXPECT_THAT(path.tiles(), testing::ElementsAre(t0, t1, t2));
  EXPECT_FALSE(path.empty());
  EXPECT_EQ(path.size(), 3);
  EXPECT_THAT(path.lowest_legal_row(), testing::ElementsAre(0, 1, 0));
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
  EXPECT_THAT(path.lowest_legal_row(), testing::ElementsAre(0, 1, 0));
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

//   b
// a
TEST(PathTest, PushBackFailsDueToDuplicateTile) {
  Path path;
  ASSERT_THAT(path.push_back(std::make_shared<Tile>(0, 0, 'a')), IsOk());
  Path unchanged = path;
  EXPECT_THAT(path.push_back(std::make_shared<Tile>(0, 0, 'b')),
              StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_EQ(path, unchanged);
}

//   b
// a
TEST(PathTest, PushBackFailsDueToColumnGap) {
  Path path;
  ASSERT_THAT(path.push_back(std::make_shared<Tile>(0, 0, 'a')), IsOk());
  Path unchanged = path;
  EXPECT_THAT(path.push_back(std::make_shared<Tile>(1, 2, 'b')),
              StatusIs(absl::StatusCode::kOutOfRange));
  EXPECT_EQ(path, unchanged);
}

// c
// a
// b
TEST(PathTest, PushBackFailsDueToInterruptedColumn) {
  Path path;
  ASSERT_THAT(path.push_back(std::make_shared<Tile>(1, 0, 'a')), IsOk());
  ASSERT_THAT(path.push_back(std::make_shared<Tile>(0, 0, 'b')), IsOk());
  Path unchanged = path;
  EXPECT_THAT(path.push_back(std::make_shared<Tile>(2, 0, 'c')),
              StatusIs(absl::StatusCode::kOutOfRange));
  EXPECT_EQ(path, unchanged);
}

// b
// c
// a
TEST(PathTest, PushBackFailsDueToNoRoom) {
  Path path;
  ASSERT_THAT(path.push_back(std::make_shared<Tile>(0, 0, 'a')), IsOk());
  ASSERT_THAT(path.push_back(std::make_shared<Tile>(2, 0, 'b')), IsOk());
  Path unchanged = path;
  EXPECT_THAT(path.push_back(std::make_shared<Tile>(1, 0, 'c')),
              StatusIs(absl::StatusCode::kOutOfRange));
  EXPECT_EQ(path, unchanged);
}

// d
// a
// bc
TEST(PathTest, PushBackFailsDueToLowestLegalRow) {
  Path path;
  ASSERT_THAT(path.push_back(std::make_shared<Tile>(1, 0, 'a')), IsOk());
  ASSERT_THAT(path.push_back(std::make_shared<Tile>(0, 0, 'b')), IsOk());
  ASSERT_THAT(path.push_back(std::make_shared<Tile>(0, 1, 'c')), IsOk());
  Path unchanged = path;
  EXPECT_THAT(path.push_back(std::make_shared<Tile>(2, 0, 'd')),
              StatusIs(absl::StatusCode::kOutOfRange));
  EXPECT_EQ(path, unchanged);
}

// b
//
// a
TEST(PathTest, PushBackSucceedsWithADrop) {
  Path path;
  ASSERT_THAT(path.push_back(std::make_shared<Tile>(0, 0, 'a')), IsOk());
  EXPECT_THAT(path.push_back(std::make_shared<Tile>(2, 0, 'b')), IsOk());
  EXPECT_EQ(path.size(), 2);
}

//  b
//
// a
TEST(PathTest, PushBackSucceedsWithAnOffsetDrop) {
  Path path;
  ASSERT_THAT(path.push_back(std::make_shared<Tile>(0, 0, 'a')), IsOk());
  EXPECT_THAT(path.push_back(std::make_shared<Tile>(2, 1, 'b')), IsOk());
  EXPECT_EQ(path.size(), 2);
}

// d
//
//  c
//  b
// ae
TEST(PathTest, AdjustedPointsAndDelta) {
  Point p00 = {0, 0};
  Point p10 = {1, 0};
  Point p30 = {3, 0};
  Point p40 = {4, 0};
  Point p01 = {0, 1};
  Point p11 = {1, 1};
  Point p21 = {2, 1};

  Path path;
  ASSERT_THAT(path.push_back(std::make_shared<Tile>(p00, 'a')), IsOk());
  ASSERT_THAT(path.push_back(std::make_shared<Tile>(p11, 'b')), IsOk());
  ASSERT_THAT(path.push_back(std::make_shared<Tile>(p21, 'c')), IsOk());
  ASSERT_THAT(path.push_back(std::make_shared<Tile>(p40, 'd')), IsOk());
  EXPECT_THAT(path.adjusted_points(),
              testing::ElementsAreArray({p00, p11, p21, p30}));
  EXPECT_EQ(path.Delta(), 1);

  EXPECT_THAT(path.push_back(std::make_shared<Tile>(p01, 'e')), IsOk());
  EXPECT_EQ(path.size(), 5);
  EXPECT_THAT(path.adjusted_points(),
              testing::ElementsAreArray({p00, p11, p21, p10, p01}));
  EXPECT_EQ(path.Delta(), 3);

  path.pop_back();
  EXPECT_EQ(path.size(), 4);
  EXPECT_THAT(path.adjusted_points(),
              testing::ElementsAreArray({p00, p11, p21, p30}));
  EXPECT_EQ(path.Delta(), 1);
}

TEST(PathTest, Word) {
  Path path;
  ASSERT_THAT(path.push_back(std::make_shared<Tile>(0, 0, 'b')), IsOk());
  ASSERT_THAT(path.push_back(std::make_shared<Tile>(0, 1, 'A')), IsOk());
  ASSERT_THAT(path.push_back(std::make_shared<Tile>(0, 2, 't')), IsOk());
  EXPECT_EQ(path.word(), "bat");
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
  EXPECT_THAT(path.lowest_legal_row(), testing::ElementsAre(0, 0));
  EXPECT_THAT(path.simple_board()[0], testing::ElementsAre(0));
  EXPECT_THAT(path.simple_board()[1], testing::ElementsAre(1));
  EXPECT_EQ(path.star_count(), 2);

  path.pop_back();
  EXPECT_THAT(path.tiles(), testing::ElementsAre(t0));
  EXPECT_FALSE(path.empty());
  EXPECT_EQ(path.size(), 1);
  EXPECT_THAT(path.lowest_legal_row(), testing::ElementsAre(0));
  EXPECT_THAT(path.simple_board()[0], testing::ElementsAre(0));
  EXPECT_EQ(path.star_count(), 1);

  path.pop_back();
  EXPECT_THAT(path.tiles(), testing::IsEmpty());
  EXPECT_TRUE(path.empty());
  EXPECT_EQ(path.size(), 0);
  EXPECT_THAT(path.lowest_legal_row(), testing::IsEmpty());
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

TEST(PathTest, AbslStringify) {
  Path path;
  EXPECT_EQ(absl::StrFormat("%v", path), "");

  ASSERT_THAT(
      path.push_back(
          {std::make_shared<Tile>(0, 0, 'q'), std::make_shared<Tile>(1, 1, 'r'),
           std::make_shared<Tile>(2, 0, 'S'), std::make_shared<Tile>(2, 1, 'T'),
           std::make_shared<Tile>(3, 2, 'u')}),
      IsOk());
  EXPECT_EQ(absl::StrFormat("%v", path),
            "q (0,0), r (1,1), S (2,0), T (2,1), u (3,2)");
}

}  // namespace
}  // namespace puzzmo::spelltower