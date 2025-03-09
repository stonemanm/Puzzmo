#include <fstream>
#include <string>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/log/log.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "src/spelltower/path.h"
#include "src/spelltower/solver.h"

using namespace puzzmo;
using ::spelltower::Path;
using ::spelltower::Solver;

ABSL_FLAG(bool, print_current_options, false,
          "Print all playable words and their scores to the command line.");

ABSL_FLAG(
    bool, print_longest_allstar_word, false,
    "Find and print the longest possible word including all star tiles to the "
    "command line.");

ABSL_FLAG(bool, solve_greedily, false,
          "Solve the Spelltower board greedily and print the solution to the "
          "command line.");

namespace {

constexpr absl::string_view kSpelltowerBoardFilePath =
    "data/spelltower_board.txt";

absl::StatusOr<Solver> LoadSolver() {
  std::ifstream file(kSpelltowerBoardFilePath);
  if (!file.is_open()) {
    return absl::InvalidArgumentError(
        absl::StrCat("Error: Could not open ", kSpelltowerBoardFilePath));
  }
  std::vector<std::string> grid_strings;
  std::string line;
  while (std::getline(file, line)) {
    grid_strings.push_back(line);
  }
  file.close();
  return Solver::CreateSolverWithSerializedDict(grid_strings);
}

}  // namespace

int main(int argc, const char *argv[]) {
  absl::StatusOr<Solver> solver = LoadSolver();
  if (!solver.ok()) {
    LOG(ERROR) << solver.status();
    return 1;
  }

  if (absl::GetFlag(FLAGS_print_current_options)) {
    solver->FillWordCache();
    auto wc = solver->word_cache();
    LOG(INFO) << "All possible words on grid: ";
    for (const auto &[score, wds] : wc) {
      LOG(INFO) << absl::StrCat(
          score, ": ",
          absl::StrJoin(wds, ", ", [](std::string *out, const Path &path) {
            absl::StrAppend(out, path.word());
          }));
    }
  }

  if (absl::GetFlag(FLAGS_print_longest_allstar_word)) {
    absl::StatusOr<Path> path = solver->LongestPossibleAllStarWord();
    if (!path.ok()) {
      LOG(ERROR) << path.status();
      return 1;
    }
    LOG(INFO) << absl::StrCat("Longest possible all-star word: ", path->word());
    for (int i = 0; i < path->size(); ++i) {
      LOG(INFO) << absl::StrCat(
          (*path)[i]->letter_on_board(), " ", (*path)[i]->coords(), " -> ",
          (*path).adjusted_points()[i], " --- (needs to drop ",
          (*path)[i]->row() - (*path).adjusted_points()[i].row, " rows)");
    }
  }

  if (absl::GetFlag(FLAGS_solve_greedily)) {
    if (absl::Status s = solver->SolveGreedily(); !s.ok()) {
      LOG(ERROR) << s;
      return 1;
    }
    LOG(INFO) << absl::StrCat("Greedy solution: \n", *solver);
  }

  return 0;
}