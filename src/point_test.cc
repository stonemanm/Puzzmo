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

}  // namespace
}  // namespace puzzmo