#include "solver.h"

#include "absl/status/status.h"
#include "absl/status/status_matchers.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace puzzmo::spelltower {
namespace {

using absl_testing::IsOk;
using absl_testing::IsOkAndHolds;
using absl_testing::StatusIs;

TEST(SolverTest, WordCache) {
  Trie trie({"carb", "crab", "arb", "arc", "bar", "bra", "cab", "car"});
  Grid grid({"cab", "..r"});
  Solver solver(trie, grid);

  EXPECT_THAT(solver.word_cache(), testing::IsEmpty());
  solver.FillWordCache();
  EXPECT_THAT(solver.word_cache(), testing::SizeIs(3));
  EXPECT_THAT(solver.word_cache().begin()->second, testing::SizeIs(1));
}

TEST(SolverTest, BestPossiblePathForWord) {
  Solver solver_with_unused_star(
      Trie({"set", "sets", "bet", "bets", "best", "bests", "test", "tests",
            "beset", "besets"}),
      Grid({"BXsx", "xxxx", "xEst", "xxxx", "bexT", "xiix", "best"}));

  // Word doesn't use a star.
  EXPECT_THAT(solver_with_unused_star.BestPossibleAllStarPathForWord("bests"),
              StatusIs(absl::StatusCode::kNotFound));

  Solver solver(Trie({"set", "sets", "bet", "bets", "best", "bests", "test",
                      "tests", "beset", "besets"}),
                Grid({"Bxsx", "xxxx", "xEst", "xxxx", "bexT", "xiix", "best"}));

  // Word not in trie.
  EXPECT_THAT(solver.BestPossibleAllStarPathForWord("exit"),
              StatusIs(absl::StatusCode::kInvalidArgument));

  Path best_path;
  ASSERT_THAT(best_path.push_back(solver.grid()[{6, 0}]), IsOk());  // B
  ASSERT_THAT(best_path.push_back(solver.grid()[{4, 1}]), IsOk());  // E
  ASSERT_THAT(best_path.push_back(solver.grid()[{6, 2}]), IsOk());  // s
  ASSERT_THAT(best_path.push_back(solver.grid()[{2, 3}]), IsOk());  // T
  ASSERT_THAT(best_path.push_back(solver.grid()[{4, 2}]), IsOk());  // s
  EXPECT_THAT(solver.BestPossibleAllStarPathForWord("bests"),
              IsOkAndHolds(best_path));
}

TEST(SolverTest, BestPossibleAllStarPathForWord) {
  Solver solver(Trie({"set", "sets", "bet", "bets", "best", "bests", "test",
                      "tests", "beset", "besets"}),
                Grid({"Bxsx", "xxxx", "xEst", "xxxx", "bexT", "xiix", "best"}));

  // Word not in trie.
  EXPECT_THAT(solver.BestPossiblePathForWord("exit"),
              StatusIs(absl::StatusCode::kInvalidArgument));

  Path best_path;
  ASSERT_THAT(best_path.push_back(solver.grid()[{6, 0}]), IsOk());  // B
  ASSERT_THAT(best_path.push_back(solver.grid()[{4, 1}]), IsOk());  // E
  ASSERT_THAT(best_path.push_back(solver.grid()[{6, 2}]), IsOk());  // s
  ASSERT_THAT(best_path.push_back(solver.grid()[{2, 3}]), IsOk());  // T
  ASSERT_THAT(best_path.push_back(solver.grid()[{4, 2}]), IsOk());  // s
  EXPECT_THAT(solver.BestPossiblePathForWord("bests"), IsOkAndHolds(best_path));
}

TEST(SolverTest, LongestPossibleAllStarWord) {
  Solver solver(Trie({"set", "sets", "bet", "bets", "best", "bests", "test",
                      "tests", "beset", "besets", "unavailable"}),
                Grid({"Bxsx", "xxxx", "xEst", "xxxx", "bexT", "xiix", "best"}));
  Path best_path;
  ASSERT_THAT(best_path.push_back(solver.grid()[{6, 0}]), IsOk());  // B
  ASSERT_THAT(best_path.push_back(solver.grid()[{4, 1}]), IsOk());  // E
  ASSERT_THAT(best_path.push_back(solver.grid()[{6, 2}]), IsOk());  // s
  ASSERT_THAT(best_path.push_back(solver.grid()[{2, 3}]), IsOk());  // T
  ASSERT_THAT(best_path.push_back(solver.grid()[{4, 2}]), IsOk());  // s

  EXPECT_THAT(solver.LongestPossibleAllStarWord(), IsOkAndHolds(best_path));
}

TEST(SolverTest, PlayWordSuccess) {
  Solver solver(
      Trie({"carb", "crab", "arb", "arc", "bar", "bra", "cab", "car"}),
      Grid({"cab", "z.r"}));

  Path word;
  ASSERT_THAT(word.push_back({solver.grid()[{1, 2}], solver.grid()[{1, 1}],
                              solver.grid()[{0, 2}]}),
              absl_testing::IsOk());  // "bar"
  EXPECT_THAT(solver.PlayWord(word), absl_testing::IsOk());
  EXPECT_FALSE(solver.grid().IsPointInRange({1, 2}));
  EXPECT_THAT(solver.solution(), testing::SizeIs(1));
  EXPECT_EQ(solver.score(), 1021);  // almost there
}

TEST(SolverTest, PlayWordFailsForEmptyPath) {
  Solver solver(
      Trie({"carb", "crab", "arb", "arc", "bar", "bra", "cab", "car"}),
      Grid({"cab", "z.r"}));

  Path word;
  EXPECT_THAT(solver.PlayWord(word),
              StatusIs(absl::StatusCode::kInvalidArgument));
}

TEST(SolverTest, PlayWordFailsForNonContinuousPath) {
  Solver solver(
      Trie({"carb", "crab", "arb", "arc", "bar", "bra", "cab", "car"}),
      Grid({"cab", "...", "z.r"}));

  Path word;
  ASSERT_THAT(word.push_back(solver.grid()[{2, 0}]), IsOk());  // c
  ASSERT_THAT(word.push_back(solver.grid()[{2, 1}]), IsOk());  // a
  ASSERT_THAT(word.push_back(solver.grid()[{0, 2}]),
              IsOk());  // r, but not continuous.
  EXPECT_THAT(solver.PlayWord(word),
              StatusIs(absl::StatusCode::kInvalidArgument));
}

TEST(SolverTest, PlayWordFailsForWordNotInTrie) {
  Solver solver(Trie({"nope", "not", "here"}), Grid({"cab"}));

  Path word;
  ASSERT_THAT(word.push_back(solver.grid()[{0, 0}]), IsOk());  // c
  ASSERT_THAT(word.push_back(solver.grid()[{0, 1}]), IsOk());  // a
  ASSERT_THAT(word.push_back(solver.grid()[{0, 2}]),
              IsOk());  // b
  EXPECT_THAT(solver.PlayWord(word),
              StatusIs(absl::StatusCode::kInvalidArgument));
}

TEST(SolverTest, SolveGreedily) {
  //
}

TEST(SolverTest, AbslStringify) {
  Trie trie({"carb", "crab", "arb", "arc", "bar", "bra", "cab", "car", "scat"});
  // sca
  // cabt.
  // ..r..
  Grid grid({"Sca", "cabT.", "..r.."});
  Solver solver(trie, grid);
  EXPECT_EQ(absl::StrFormat("%v", solver),
            absl::StrFormat("%v", solver.grid()));

  Path scat;
  ASSERT_THAT(scat.push_back({solver.grid()[{2, 0}], solver.grid()[{2, 1}],
                              solver.grid()[{2, 2}], solver.grid()[{1, 3}]}),
              absl_testing::IsOk());
  std::string scat_str =
      absl::StrCat("1. \"scat\"\n", solver.grid().VisualizePath(scat));
  ASSERT_THAT(solver.PlayWord(scat), absl_testing::IsOk());
  EXPECT_EQ(
      absl::StrFormat("%v", solver),
      absl::StrCat(scat_str, "\n\n", absl::StrFormat("%v", solver.grid())));

  Path carb;
  ASSERT_THAT(carb.push_back({solver.grid()[{1, 0}], solver.grid()[{1, 1}],
                              solver.grid()[{0, 2}], solver.grid()[{1, 2}]}),
              absl_testing::IsOk());
  std::string carb_str =
      absl::StrCat("2. \"carb\"\n", solver.grid().VisualizePath(carb));
  ASSERT_THAT(solver.PlayWord(carb), absl_testing::IsOk());
  EXPECT_EQ(absl::StrFormat("%v", solver),
            absl::StrCat(scat_str, "\n\n", carb_str, "\n\n",
                         absl::StrFormat("%v", solver.grid())));
}

}  // namespace
}  // namespace puzzmo::spelltower