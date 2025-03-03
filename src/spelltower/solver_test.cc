#include "solver.h"

#include <memory>

#include "absl/status/status_matchers.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace puzzmo::spelltower {
namespace {

TEST(SolverTest, WordCache) {
  Trie trie({"carb", "crab", "arb", "arc", "bar", "bra", "cab", "car"});
  Grid grid({"cab", "..r"});
  Solver solver(trie, grid);

  EXPECT_THAT(solver.word_cache(), testing::IsEmpty());
  solver.FillWordCache();
  EXPECT_THAT(solver.word_cache(), testing::SizeIs(3));
  EXPECT_THAT(solver.word_cache().begin()->second, testing::SizeIs(1));
}

TEST(SolverTest, PlayWord) {
  Solver solver(
      Trie({"carb", "crab", "arb", "arc", "bar", "bra", "cab", "car"}),
      Grid({"cab", "z.r"}));

  Path word({solver.grid()[{1, 2}], solver.grid()[{1, 1}],
             solver.grid()[{0, 2}]});  // "bar"
  EXPECT_THAT(solver.PlayWord(word), absl_testing::IsOk());
  EXPECT_FALSE(solver.grid().IsPointInRange({1, 2}));
  EXPECT_THAT(solver.solution(), testing::SizeIs(1));
  EXPECT_EQ(solver.score(), 1021);  // almost there
}

TEST(SolverTest, SolveGreedily) {
  //
}

TEST(SolverTest, AbslStringify) {
  Trie trie({"carb", "crab", "arb", "arc", "bar", "bra", "cab", "car", "scat"});
  // sca
  // cabt.
  // ..r..
  Grid grid({"sca", "cabt.", "..r.."});
  Solver solver(trie, grid);
  EXPECT_EQ(absl::StrFormat("%v", solver),
            absl::StrFormat("%v", solver.grid()));

  Path scat({solver.grid()[{2, 0}], solver.grid()[{2, 1}],
             solver.grid()[{2, 2}], solver.grid()[{1, 3}]});
  ASSERT_THAT(solver.PlayWord(scat), absl_testing::IsOk());
  EXPECT_EQ(
      absl::StrFormat("%v", solver),
      absl::StrCat("1. ", scat, "\n\n", absl::StrFormat("%v", solver.grid())));

  Path carb({solver.grid()[{1, 0}], solver.grid()[{1, 1}],
             solver.grid()[{0, 2}], solver.grid()[{1, 2}]});
  ASSERT_THAT(solver.PlayWord(carb), absl_testing::IsOk());
  EXPECT_EQ(absl::StrFormat("%v", solver),
            absl::StrCat("1. ", scat, "\n\n", "2. ", carb, "\n\n",
                         absl::StrFormat("%v", solver.grid())));
}

}  // namespace
}  // namespace puzzmo::spelltower