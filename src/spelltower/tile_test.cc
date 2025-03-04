#include "tile.h"

#include "absl/status/status_matchers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace puzzmo::spelltower {
namespace {

TEST(TileTest, Constructor) {
  Tile nonalpha_tile(0, 0, '$');
  EXPECT_THAT(nonalpha_tile.is_blank(), testing::IsTrue());
  EXPECT_THAT(nonalpha_tile.is_rare(), testing::IsFalse());
  EXPECT_THAT(nonalpha_tile.is_star(), testing::IsFalse());
  EXPECT_EQ(nonalpha_tile.value(), 0);

  Tile letter_tile(0, 1, 'a');
  EXPECT_THAT(letter_tile.is_blank(), testing::IsFalse());
  EXPECT_THAT(letter_tile.is_rare(), testing::IsFalse());
  EXPECT_THAT(letter_tile.is_star(), testing::IsFalse());
  EXPECT_EQ(letter_tile.value(), 1);

  Tile rare_tile(0, 2, 'j');
  EXPECT_THAT(rare_tile.is_blank(), testing::IsFalse());
  EXPECT_THAT(rare_tile.is_rare(), testing::IsTrue());
  EXPECT_THAT(rare_tile.is_star(), testing::IsFalse());
  EXPECT_EQ(rare_tile.value(), 9);

  Tile star_tile(0, 3, 'A');
  EXPECT_THAT(star_tile.is_blank(), testing::IsFalse());
  EXPECT_THAT(star_tile.is_rare(), testing::IsFalse());
  EXPECT_THAT(star_tile.is_star(), testing::IsTrue());
  EXPECT_EQ(star_tile.value(), 1);

  Tile rare_star_tile(0, 4, 'J');
  EXPECT_THAT(rare_star_tile.is_blank(), testing::IsFalse());
  EXPECT_THAT(rare_star_tile.is_rare(), testing::IsTrue());
  EXPECT_THAT(rare_star_tile.is_star(), testing::IsTrue());
  EXPECT_EQ(rare_star_tile.value(), 9);
}

TEST(TileTest, Drop) {
  Tile tile(5, 3);
  EXPECT_THAT(tile.Drop(4), absl_testing::IsOk());
  EXPECT_THAT(tile.Drop(4),
              absl_testing::StatusIs(absl::StatusCode::kOutOfRange));
}

TEST(TileTest, AbslStringify) {
  EXPECT_EQ(absl::StrFormat("%v", Tile(0, 0)),
            std::string(1, kBlankTileLetter));

  EXPECT_EQ(absl::StrFormat("%v", Tile(0, 0, 'a')), "a");

  EXPECT_EQ(absl::StrFormat("%v", Tile(0, 0, 'A')), "A");
}

}  // namespace
}  // namespace puzzmo::spelltower