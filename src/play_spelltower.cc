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

//-------
// Files

ABSL_FLAG(std::string, spelltower_board_file_path, "data/spelltower_board.txt",
          "Path to the input file containing a string representation of the "
          "board, with star letters capitalized.");

//------------
// Parameters

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

//---------
// Modules

absl::StatusOr<Solver> LoadSolver() {
  std::ifstream file(absl::GetFlag(FLAGS_spelltower_board_file_path));
  if (!file.is_open()) {
    return absl::InvalidArgumentError(
        absl::StrCat("Error: Could not open ",
                     absl::GetFlag(FLAGS_spelltower_board_file_path)));
  }
  std::vector<std::string> grid_strings;
  std::string line;
  while (std::getline(file, line)) {
    grid_strings.push_back(line);
  }
  file.close();
  return Solver::CreateSolverWithSerializedDict(grid_strings);
}

void PrintCurrentOptions(const Solver &solver) {
  absl::btree_map<int, absl::btree_set<Path>, std::greater<int>> wc =
      solver.word_cache();
  LOG(INFO) << "All possible words on grid: ";
  for (const auto &[score, wds] : wc) {
    LOG(INFO) << absl::StrCat(
        score, ": ",
        absl::StrJoin(wds, ", ", [](std::string *out, const Path &path) {
          absl::StrAppend(out, path.word());
        }));
  }
}

absl::Status PrintLongestAllStarWord(const Solver &solver) {
  absl::StatusOr<Path> path = solver.BestPossibleGoalWord();
  if (!path.ok()) return path.status();

  LOG(INFO) << absl::StrCat("Best possible goal word: ", path->word());
  for (int i = 0; i < path->size(); ++i) {
    LOG(INFO) << absl::StrCat(
        (*path)[i]->letter_on_board(), " ", (*path)[i]->coords(), " -> ",
        (*path).adjusted_points()[i], " --- (needs to drop ",
        (*path)[i]->row() - (*path).adjusted_points()[i].row, " rows)");
  }
}

}  // namespace

//------
// main

int main(int argc, const char *argv[]) {
  absl::StatusOr<Solver> solver = LoadSolver();
  if (!solver.ok()) {
    LOG(ERROR) << solver.status();
    return 1;
  }

  if (absl::GetFlag(FLAGS_print_current_options)) {
    solver->FillWordCache();
    PrintCurrentOptions(*solver);
  }

  if (absl::GetFlag(FLAGS_print_longest_allstar_word)) {
    if (absl::Status s = PrintLongestAllStarWord(*solver); !s.ok()) {
      LOG(ERROR) << s;
      return 1;
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