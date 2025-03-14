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

// TEST(SolverTest, BestPossibleThreeStarPathForWord) {
//   Solver solver_with_unused_star(
//       Trie({"ests", "set", "sets", "bet", "bets", "best", "bests", "test",
//             "tests", "beset", "besets"}),
//       Grid({"Bxsx", "xxxx", "xEst", "xxxx", "bexT", "xiix", "best"}));

//   // Word doesn't use a star.
//   EXPECT_THAT(solver_with_unused_star.BestPossibleThreeStarPathForWord("ests"),
//               StatusIs(absl::StatusCode::kNotFound));

//   Solver solver(Trie({"set", "sets", "bet", "bets", "best", "bests", "test",
//                       "tests", "beset", "besets"}),
//                 Grid({"Bxsx", "xxxx", "xEst", "xxxx", "bexT", "xiix",
//                 "best"}));

//   // Word not in trie.
//   EXPECT_THAT(solver.BestPossibleThreeStarPathForWord("exit"),
//               StatusIs(absl::StatusCode::kInvalidArgument));

//   Path best_path;
//   ASSERT_THAT(best_path.push_back(solver.TileAt(6, 0)), IsOk());  // B
//   ASSERT_THAT(best_path.push_back(solver.TileAt(4, 1)), IsOk());  // E
//   ASSERT_THAT(best_path.push_back(solver.TileAt(6, 2)), IsOk());  // s
//   ASSERT_THAT(best_path.push_back(solver.TileAt(2, 3)), IsOk());  // T
//   ASSERT_THAT(best_path.push_back(solver.TileAt(4, 2)), IsOk());  // s
//   EXPECT_THAT(solver.BestPossibleThreeStarPathForWord("bests"),
//               IsOkAndHolds(best_path));
// }

// TEST(SolverTest, BestPossiblePathForWord) {
//   Solver solver(Trie({"set", "sets", "bet", "bets", "best", "bests", "test",
//                       "tests", "beset", "besets"}),
//                 Grid({"Bxsx", "xxxx", "xEst", "xxxx", "bexT", "xiix",
//                 "best"}));

//   // Word not in trie.
//   EXPECT_THAT(solver.BestPossiblePathForWord("exit"),
//               StatusIs(absl::StatusCode::kInvalidArgument));

//   Path best_path;
//   ASSERT_THAT(best_path.push_back(solver.TileAt(6, 0)), IsOk());  // B
//   ASSERT_THAT(best_path.push_back(solver.TileAt(4, 1)), IsOk());  // E
//   ASSERT_THAT(best_path.push_back(solver.TileAt(6, 2)), IsOk());  // s
//   ASSERT_THAT(best_path.push_back(solver.TileAt(2, 3)), IsOk());  // T
//   ASSERT_THAT(best_path.push_back(solver.TileAt(4, 2)), IsOk());  // s
//   EXPECT_THAT(solver.BestPossiblePathForWord("bests"),
//   IsOkAndHolds(best_path));
// }

// TEST(SolverTest, BestPossibleGoalWord) {
//   Solver solver(Trie({"set", "sets", "bet", "bets", "best", "bests", "test",
//                       "tests", "beset", "besets", "unavailable"}),
//                 Grid({"Bxsx", "xxxx", "xEst", "xxxx", "bexT", "xiix",
//                 "best"}));
//   Path best_path;
//   ASSERT_THAT(best_path.push_back(solver.TileAt(6, 0)), IsOk());  // B
//   ASSERT_THAT(best_path.push_back(solver.TileAt(4, 1)), IsOk());  // E
//   ASSERT_THAT(best_path.push_back(solver.TileAt(6, 2)), IsOk());  // s
//   ASSERT_THAT(best_path.push_back(solver.TileAt(2, 3)), IsOk());  // T
//   ASSERT_THAT(best_path.push_back(solver.TileAt(4, 2)), IsOk());  // s

//   EXPECT_THAT(solver.BestPossibleGoalWord(), IsOkAndHolds(best_path));
// }

TEST(SolverTest, PlayWordSuccess) {
  Solver solver(
      Trie({"carb", "crab", "arb", "arc", "bar", "bra", "cab", "car"}),
      Grid({"cab", "z.r"}));

  Path word;
  ASSERT_THAT(word.push_back({solver.TileAt(1, 2), solver.TileAt(1, 1),
                              solver.TileAt(0, 2)}),
              absl_testing::IsOk());  // "bar"
  EXPECT_THAT(solver.PlayWord(word), absl_testing::IsOk());
  EXPECT_FALSE(solver.grid().IsPointInRange({1, 2}));
  EXPECT_THAT(solver.solution(), testing::SizeIs(1));
  EXPECT_EQ(solver.score(), 1021);  // almost there
}

TEST(SolverTest, UndoLastPlay) {
  Solver solver(
      Trie({"carb", "crab", "arb", "arc", "bar", "bra", "cab", "car"}),
      Grid({"cab", "z.r"}));
  auto starting_grid_vecs = solver.grid().tiles();

  Path word;
  ASSERT_THAT(word.push_back({solver.TileAt(1, 2), solver.TileAt(1, 1),
                              solver.TileAt(0, 2)}),
              absl_testing::IsOk());  // "bar"
  ASSERT_THAT(solver.PlayWord(word), IsOk());

  EXPECT_THAT(solver.UndoLastPlay(), IsOk());
  EXPECT_EQ(solver.grid().tiles(), starting_grid_vecs);
  EXPECT_EQ(solver.score(), 1000);
  EXPECT_THAT(solver.word_cache().empty(), testing::IsTrue());
  EXPECT_THAT(solver.solution().empty(), testing::IsTrue());
  EXPECT_THAT(solver.snapshots().empty(), testing::IsTrue());

  EXPECT_THAT(solver.UndoLastPlay(),
              StatusIs(absl::StatusCode::kFailedPrecondition));
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
  ASSERT_THAT(word.push_back(solver.TileAt(2, 0)), IsOk());  // c
  ASSERT_THAT(word.push_back(solver.TileAt(2, 1)), IsOk());  // a
  ASSERT_THAT(word.push_back(solver.TileAt(0, 2)),
              IsOk());  // r, but not continuous.
  EXPECT_THAT(solver.PlayWord(word),
              StatusIs(absl::StatusCode::kInvalidArgument));
}

TEST(SolverTest, PlayWordFailsForWordNotInTrie) {
  Solver solver(Trie({"nope", "not", "here"}), Grid({"cab"}));

  Path word;
  ASSERT_THAT(word.push_back(solver.TileAt(0, 0)), IsOk());  // c
  ASSERT_THAT(word.push_back(solver.TileAt(0, 1)), IsOk());  // a
  ASSERT_THAT(word.push_back(solver.TileAt(0, 2)),
              IsOk());  // b
  EXPECT_THAT(solver.PlayWord(word),
              StatusIs(absl::StatusCode::kInvalidArgument));
}

TEST(SolverTest, SolveWithOneLongWordSuccess) {
  Solver solver(
      Trie({"successfully", "tiny", "word", "greed"}),
      Grid({"   lluf", "   tond", "  ywiry", "SUCcess", ".......", ".greed."}));
  Path tiny;
  ASSERT_THAT(tiny.push_back({
                  solver.TileAt(4, 3),  // t
                  solver.TileAt(3, 4),  // i
                  solver.TileAt(4, 5),  // n
                  solver.TileAt(3, 6),  // y
              }),
              IsOk());
  Path word;
  ASSERT_THAT(word.push_back({
                  solver.TileAt(3, 3),  // w
                  solver.TileAt(4, 4),  // o
                  solver.TileAt(3, 5),  // r
                  solver.TileAt(4, 6),  // d
              }),
              IsOk());
  Path successfully;
  ASSERT_THAT(successfully.push_back({
                  solver.TileAt(2, 0),  // s
                  solver.TileAt(2, 1),  // u
                  solver.TileAt(2, 2),  // c
                  solver.TileAt(2, 3),  // c
                  solver.TileAt(2, 4),  // e
                  solver.TileAt(2, 5),  // s
                  solver.TileAt(2, 6),  // s
                  solver.TileAt(5, 6),  // f
                  solver.TileAt(5, 5),  // u
                  solver.TileAt(5, 4),  // l
                  solver.TileAt(5, 3),  // l
                  solver.TileAt(3, 2)   // y
              }),
              IsOk());
  Path greed;
  ASSERT_THAT(greed.push_back({
                  solver.TileAt(0, 1),  // g
                  solver.TileAt(0, 2),  // r
                  solver.TileAt(0, 3),  // e
                  solver.TileAt(0, 4),  // e
                  solver.TileAt(0, 5),  // d
              }),
              IsOk());
  EXPECT_THAT(solver.SolveWithOneLongWord(), IsOk());
  EXPECT_THAT(solver.solution(),
              testing::UnorderedElementsAre(word, tiny, successfully, greed));
}

TEST(SolverTest, PlayGoalWordFails) {
  Solver solver(Trie({"successfully", "longer", "word"}),
                Grid({"   lluf", "  onger", " lyword", "success"}));
  Path longer;
  ASSERT_THAT(longer.push_back({
                  solver.TileAt(1, 1),  // l
                  solver.TileAt(2, 2),  // o
                  solver.TileAt(2, 3),  // n
                  solver.TileAt(2, 4),  // g
                  solver.TileAt(2, 5),  // e
                  solver.TileAt(2, 6),  // r
              }),
              IsOk());
  Path word;
  ASSERT_THAT(word.push_back({
                  solver.TileAt(1, 3),  // w
                  solver.TileAt(1, 4),  // o
                  solver.TileAt(1, 5),  // r
                  solver.TileAt(1, 6),  // d
              }),
              IsOk());
  Path goal_word;
  ASSERT_THAT(goal_word.push_back({
                  solver.TileAt(0, 0),  // s
                  solver.TileAt(0, 1),  // u
                  solver.TileAt(0, 2),  // c
                  solver.TileAt(0, 3),  // c
                  solver.TileAt(0, 4),  // e
                  solver.TileAt(0, 5),  // s
                  solver.TileAt(0, 6),  // s
                  solver.TileAt(3, 6),  // f
                  solver.TileAt(3, 5),  // u
                  solver.TileAt(3, 4),  // l
                  solver.TileAt(3, 3),  // l
                  solver.TileAt(1, 2)   // y
              }),
              IsOk());
  EXPECT_THAT(solver.PlayGoalWord(goal_word),
              StatusIs(absl::StatusCode::kNotFound));
  EXPECT_THAT(solver.solution(), testing::SizeIs(0));
}

TEST(SolverTest, SolveGreedily) {}

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
  ASSERT_THAT(scat.push_back({solver.TileAt(2, 0), solver.TileAt(2, 1),
                              solver.TileAt(2, 2), solver.TileAt(1, 3)}),
              absl_testing::IsOk());
  std::string scat_str =
      absl::StrCat("1. \"scat\"\n", solver.grid().VisualizePath(scat));
  ASSERT_THAT(solver.PlayWord(scat), absl_testing::IsOk());
  EXPECT_EQ(
      absl::StrFormat("%v", solver),
      absl::StrCat(scat_str, "\n\n", absl::StrFormat("%v", solver.grid())));

  Path carb;
  ASSERT_THAT(carb.push_back({solver.TileAt(1, 0), solver.TileAt(1, 1),
                              solver.TileAt(0, 2), solver.TileAt(1, 2)}),
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