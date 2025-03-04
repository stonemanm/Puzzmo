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

  Tile letter_tile(0, 1, 'a');
  EXPECT_THAT(letter_tile.is_blank(), testing::IsFalse());
  EXPECT_THAT(letter_tile.is_rare(), testing::IsFalse());
  EXPECT_THAT(letter_tile.is_star(), testing::IsFalse());

  Tile rare_tile(0, 2, 'j');
  EXPECT_THAT(rare_tile.is_blank(), testing::IsFalse());
  EXPECT_THAT(rare_tile.is_rare(), testing::IsTrue());
  EXPECT_THAT(rare_tile.is_star(), testing::IsFalse());

  Tile star_tile(0, 3, 'A');
  EXPECT_THAT(star_tile.is_blank(), testing::IsFalse());
  EXPECT_THAT(star_tile.is_rare(), testing::IsFalse());
  EXPECT_THAT(star_tile.is_star(), testing::IsTrue());

  Tile rare_star_tile(0, 4, 'J');
  EXPECT_THAT(rare_star_tile.is_blank(), testing::IsFalse());
  EXPECT_THAT(rare_star_tile.is_rare(), testing::IsTrue());
  EXPECT_THAT(rare_star_tile.is_star(), testing::IsTrue());
}

TEST(TileTest, Drop) {
  Tile tile(5, 3);
  EXPECT_THAT(tile.Drop(4), absl_testing::IsOk());
  EXPECT_THAT(tile.Drop(4),
              absl_testing::StatusIs(absl::StatusCode::kOutOfRange));
}

TEST(TileTest, AbslStringify) {
  Tile tile(0, 0);
  EXPECT_EQ(absl::StrFormat("a %v z", tile),
            absl::StrCat("a ", std::string(1, kBlankTileLetter), " z"));

  tile.set_letter('a');
  EXPECT_EQ(absl::StrFormat("a %v z", tile), "a a z");

  tile.set_is_star(true);
  EXPECT_EQ(absl::StrFormat("a %v z", tile), "a A z");
}

}  // namespace
}  // namespace puzzmo::spelltower