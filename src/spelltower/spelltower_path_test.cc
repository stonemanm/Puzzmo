#include "spelltower_path.h"

#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace puzzmo {
namespace {

constexpr Point p00 = {0, 0};
constexpr Point p11 = {1, 1};
constexpr Point p20 = {2, 0};
constexpr Point p22 = {2, 2};
constexpr Point p33 = {3, 3};

TEST(SpelltowerPathTest, EmptyConstructor) {
  SpelltowerPath path;
  EXPECT_THAT(path.points(), testing::IsEmpty());
  EXPECT_TRUE(path.empty());
  EXPECT_EQ(path.size(), 0);
  EXPECT_THAT(path.num_below(), testing::IsEmpty());
  EXPECT_THAT(path.simplified_board(), testing::SizeIs(9));
  EXPECT_THAT(path.SimplifiedRow(0), testing::IsEmpty());
  EXPECT_THAT(path.IndicesByColumn(), testing::IsEmpty());
}

TEST(SpelltowerPathTest, VectorConstructor) {
  SpelltowerPath path({p11, p00, p33, p22});
  EXPECT_THAT(path.points(), testing::ElementsAre(p11, p00, p33, p22));
  EXPECT_FALSE(path.empty());
  EXPECT_THAT(path.num_below(), testing::ElementsAre(0, 0, 0, 0));
  EXPECT_THAT(path.simplified_board(),
              testing::ElementsAre(
                  testing::ElementsAre(1), testing::ElementsAre(0),
                  testing::ElementsAre(3), testing::ElementsAre(2),
                  testing::IsEmpty(), testing::IsEmpty(), testing::IsEmpty(),
                  testing::IsEmpty(), testing::IsEmpty()));
  EXPECT_THAT(path.IndicesByColumn(), testing::ElementsAre(1, 0, 3, 2));
}

TEST(SpelltowerPathTest, PushBack) {
  SpelltowerPath path;

  path.push_back(p22);
  EXPECT_THAT(path.points(), testing::ElementsAre(p22));
  EXPECT_FALSE(path.empty());
  EXPECT_EQ(path.size(), 1);
  EXPECT_THAT(path.num_below(), testing::ElementsAre(0));
  EXPECT_THAT(path.SimplifiedRow(2), testing::ElementsAre(0));
  EXPECT_THAT(path.SimplifiedRow(0), testing::IsEmpty());

  path.push_back(p00);
  EXPECT_THAT(path.points(), testing::ElementsAre(p22, p00));
  EXPECT_THAT(path.num_below(), testing::ElementsAre(0, 0));
  EXPECT_THAT(path.SimplifiedRow(2), testing::ElementsAre(0));
  EXPECT_THAT(path.SimplifiedRow(0), testing::ElementsAre(1));

  path.push_back(p20);
  EXPECT_THAT(path.points(), testing::ElementsAre(p22, p00, p20));
  EXPECT_THAT(path.num_below(), testing::ElementsAre(1, 0, 0));
  EXPECT_THAT(path.SimplifiedRow(2), testing::ElementsAre(2, 0));
  EXPECT_THAT(path.SimplifiedRow(0), testing::ElementsAre(1));
}

TEST(SpelltowerPathTest, PopBack) {
  SpelltowerPath path({p22, p00, p20, p33});

  path.pop_back();
  EXPECT_THAT(path.points(), testing::ElementsAre(p22, p00, p20));
  EXPECT_THAT(path.num_below(), testing::ElementsAre(1, 0, 0));
  EXPECT_THAT(path.SimplifiedRow(2), testing::ElementsAre(2, 0));
  EXPECT_THAT(path.SimplifiedRow(0), testing::ElementsAre(1));
  EXPECT_THAT(path.SimplifiedRow(3), testing::IsEmpty());

  path.pop_back();
  EXPECT_THAT(path.points(), testing::ElementsAre(p22, p00));
  EXPECT_THAT(path.num_below(), testing::ElementsAre(0, 0));
  EXPECT_THAT(path.SimplifiedRow(2), testing::ElementsAre(0));
  EXPECT_THAT(path.SimplifiedRow(0), testing::ElementsAre(1));

  path.pop_back();
  EXPECT_THAT(path.points(), testing::ElementsAre(p22));
  EXPECT_THAT(path.num_below(), testing::ElementsAre(0));
  EXPECT_THAT(path.SimplifiedRow(2), testing::ElementsAre(0));
  EXPECT_THAT(path.SimplifiedRow(0), testing::IsEmpty());
}

}  // namespace
}  // namespace puzzmo