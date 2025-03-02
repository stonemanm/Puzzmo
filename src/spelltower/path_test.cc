#include "path.h"

#include <memory>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace puzzmo::spelltower {
namespace {

TEST(PathTest, EmptyConstructor) {
  Path path;
  EXPECT_THAT(path.tiles(), testing::IsEmpty());
  EXPECT_TRUE(path.empty());
  EXPECT_EQ(path.size(), 0);
  EXPECT_THAT(path.min_possible_row(), testing::IsEmpty());
  EXPECT_THAT(path.simple_board(), testing::SizeIs(9));
  EXPECT_THAT(path.simple_board()[0], testing::IsEmpty());
  EXPECT_EQ(path.star_count(), 0);
}

TEST(PathTest, VectorConstructor) {
  std::shared_ptr<Tile> t0 = std::make_shared<Tile>(0, 0, 'A');
  std::shared_ptr<Tile> t1 = std::make_shared<Tile>(1, 1, 'B');
  std::shared_ptr<Tile> t2 = std::make_shared<Tile>(0, 1, 'c');
  Path path({t0, t1, t2});

  EXPECT_THAT(path.tiles(), testing::ElementsAre(t0, t1, t2));
  EXPECT_FALSE(path.empty());
  EXPECT_EQ(path.size(), 3);
  EXPECT_THAT(path.min_possible_row(), testing::ElementsAre(0, 1, 0));
  EXPECT_THAT(path.simple_board(),
              testing::ElementsAre(
                  testing::ElementsAre(0), testing::ElementsAre(2, 1),
                  testing::IsEmpty(), testing::IsEmpty(), testing::IsEmpty(),
                  testing::IsEmpty(), testing::IsEmpty(), testing::IsEmpty(),
                  testing::IsEmpty()));
  EXPECT_EQ(path.star_count(), 2);
}

TEST(PathTest, TilesAsString) {
  std::shared_ptr<Tile> t0 = std::make_shared<Tile>(0, 0, 'A');
  std::shared_ptr<Tile> t1 = std::make_shared<Tile>(1, 1, 'B');
  std::shared_ptr<Tile> t2 = std::make_shared<Tile>(0, 1, 'c');
  Path path({t0, t1, t2});

  EXPECT_EQ(path.TilesAsString(), "ABc");
}

TEST(PathTest, PushBack) {
  std::shared_ptr<Tile> t0 = std::make_shared<Tile>(0, 0, 'A');
  std::shared_ptr<Tile> t1 = std::make_shared<Tile>(1, 1, 'B');
  std::shared_ptr<Tile> t2 = std::make_shared<Tile>(0, 1, 'c');
  Path path;

  path.push_back(t0);
  EXPECT_THAT(path.tiles(), testing::ElementsAre(t0));
  EXPECT_FALSE(path.empty());
  EXPECT_EQ(path.size(), 1);
  EXPECT_THAT(path.min_possible_row(), testing::ElementsAre(0));
  EXPECT_THAT(path.simple_board()[0], testing::ElementsAre(0));
  EXPECT_EQ(path.star_count(), 1);

  path.push_back(t1);
  EXPECT_THAT(path.tiles(), testing::ElementsAre(t0, t1));
  EXPECT_FALSE(path.empty());
  EXPECT_EQ(path.size(), 2);
  EXPECT_THAT(path.min_possible_row(), testing::ElementsAre(0, 0));
  EXPECT_THAT(path.simple_board()[0], testing::ElementsAre(0));
  EXPECT_THAT(path.simple_board()[1], testing::ElementsAre(1));
  EXPECT_EQ(path.star_count(), 2);

  path.push_back(t2);
  EXPECT_THAT(path.tiles(), testing::ElementsAre(t0, t1, t2));
  EXPECT_FALSE(path.empty());
  EXPECT_EQ(path.size(), 3);
  EXPECT_THAT(path.min_possible_row(), testing::ElementsAre(0, 1, 0));
  EXPECT_THAT(path.simple_board()[0], testing::ElementsAre(0));
  EXPECT_THAT(path.simple_board()[1], testing::ElementsAre(2, 1));
  EXPECT_EQ(path.star_count(), 2);
}

TEST(PathTest, PopBack) {
  std::shared_ptr<Tile> t0 = std::make_shared<Tile>(0, 0, 'A');
  std::shared_ptr<Tile> t1 = std::make_shared<Tile>(1, 1, 'B');
  std::shared_ptr<Tile> t2 = std::make_shared<Tile>(0, 1, 'c');
  Path path({t0, t1, t2});

  path.pop_back();
  EXPECT_THAT(path.tiles(), testing::ElementsAre(t0, t1));
  EXPECT_FALSE(path.empty());
  EXPECT_EQ(path.size(), 2);
  EXPECT_THAT(path.min_possible_row(), testing::ElementsAre(0, 0));
  EXPECT_THAT(path.simple_board()[0], testing::ElementsAre(0));
  EXPECT_THAT(path.simple_board()[1], testing::ElementsAre(1));
  EXPECT_EQ(path.star_count(), 2);

  path.pop_back();
  EXPECT_THAT(path.tiles(), testing::ElementsAre(t0));
  EXPECT_FALSE(path.empty());
  EXPECT_EQ(path.size(), 1);
  EXPECT_THAT(path.min_possible_row(), testing::ElementsAre(0));
  EXPECT_THAT(path.simple_board()[0], testing::ElementsAre(0));
  EXPECT_EQ(path.star_count(), 1);

  path.pop_back();
  EXPECT_THAT(path.tiles(), testing::IsEmpty());
  EXPECT_TRUE(path.empty());
  EXPECT_EQ(path.size(), 0);
  EXPECT_THAT(path.min_possible_row(), testing::IsEmpty());
  EXPECT_THAT(path.simple_board()[0], testing::IsEmpty());
  EXPECT_EQ(path.star_count(), 0);
}

TEST(PathTest, IsPossible) {
  std::shared_ptr<Tile> t00 = std::make_shared<Tile>(0, 0);
  std::shared_ptr<Tile> t10 = std::make_shared<Tile>(1, 0);
  std::shared_ptr<Tile> t20 = std::make_shared<Tile>(2, 0);
  std::shared_ptr<Tile> t30 = std::make_shared<Tile>(3, 0);
  std::shared_ptr<Tile> t01 = std::make_shared<Tile>(0, 1);
  std::shared_ptr<Tile> t11 = std::make_shared<Tile>(1, 1);
  std::shared_ptr<Tile> t21 = std::make_shared<Tile>(2, 1);
  std::shared_ptr<Tile> t12 = std::make_shared<Tile>(1, 2);

  Path empty_path;
  EXPECT_TRUE(empty_path.IsPossible());

  //   2
  // 01
  Path simple_and_true({t00, t01, t12});
  EXPECT_TRUE(simple_and_true.IsPossible());

  //   1
  // 0
  Path false_bc_column_gap({t00, t12});
  EXPECT_FALSE(false_bc_column_gap.IsPossible());

  // 1
  //
  // 0
  Path simple_drop({t00, t20});
  EXPECT_TRUE(simple_drop.IsPossible());

  //  1
  //
  // 0
  Path simple_drop_offset({t00, t21});
  EXPECT_TRUE(simple_drop_offset.IsPossible());

  // 1
  // 2
  // 0
  Path false_bc_interrupted_column({t00, t20, t10});
  EXPECT_FALSE(false_bc_interrupted_column.IsPossible());

  // 3
  // 0
  // 12
  Path false_bc_tiles_beneath({t10, t00, t01, t20});
  EXPECT_FALSE(false_bc_tiles_beneath.IsPossible());

  // 4
  //  3
  //  52
  // 01
  Path complicated_and_true({t00, t01, t12, t21, t30, t11});
  EXPECT_TRUE(complicated_and_true.IsPossible());

  // 6
  // 54
  // 073
  // 12
  Path complicated_and_false({t10, t00, t01, t12, t21, t20, t30, t11});
  EXPECT_FALSE(complicated_and_false.IsPossible());
}

}  // namespace
}  // namespace puzzmo::spelltower