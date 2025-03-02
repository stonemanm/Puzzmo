#include "tile.h"

#include "absl/status/status_matchers.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace puzzmo::spelltower {
namespace {

TEST(TileTest, Constructor) {
  Tile star_tile(0, 0, 'A');
  EXPECT_TRUE(star_tile.is_star());
  Tile not_a_star_tile(0, 1, 'a');
  EXPECT_FALSE(not_a_star_tile.is_star());
  Tile blank_tile(0, 2, '*');
  EXPECT_FALSE(blank_tile.is_star());
}

TEST(TileTest, Drop) {
  Tile tile(5, 3);
  EXPECT_THAT(tile.Drop(4), absl_testing::IsOk());
  EXPECT_THAT(tile.Drop(4),
              absl_testing::StatusIs(absl::StatusCode::kOutOfRange));
}

}  // namespace
}  // namespace puzzmo::spelltower