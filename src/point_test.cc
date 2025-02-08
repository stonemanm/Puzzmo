#include "point.h"

#include "gtest/gtest.h"

namespace puzzmo {
namespace {

TEST(PointTest, VonNeumannNeighbors) {
  const Point &p = {.col = 1, .row = 2};
  EXPECT_EQ(p.VonNeumannNeighbors().size(), 4);
}

} // namespace
} // namespace puzzmo