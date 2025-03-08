#include "point.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace puzzmo {
namespace {

TEST(PointTest, VonNeumannNeighbors) {
  const Point &p = {.col = 1, .row = 2};
  EXPECT_EQ(p.VonNeumannNeighbors().size(), 4);
}

TEST(PointTest, MooreNeighbors) {
  const Point &p = {.col = 1, .row = 2};
  EXPECT_THAT(p.MooreNeighbors().size(), 8);
}

TEST(PointTest, AbslStringify) {
  Point p = {.row = 3, .col = 2};
  EXPECT_EQ(absl::StrFormat("%v", p), "(3,2)");
}

}  // namespace
}  // namespace puzzmo