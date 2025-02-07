#include "point.h"

#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace puzzmo {
namespace {

using ::testing::Eq;

TEST(PointTest, VonNeumannNeighbors) {
  const Point &p = {.col = 1, .row = 2};
  EXPECT_EQ(p.VonNeumannNeighbors().size(), 4);
}

} // namespace
} // namespace puzzmo