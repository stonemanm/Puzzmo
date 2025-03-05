#include <fstream>
#include <string>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/log/log.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "src/spelltower/solver.h"

using namespace puzzmo;
using ::spelltower::Solver;

ABSL_FLAG(bool, run_regex, true, "Run regex mode instead of the DFS?");

ABSL_FLAG(std::string, path_to_board_file, "data/spelltower_board.txt",
          "Input file containing up to a 9x13 char grid.");

namespace {

absl::StatusOr<std::vector<std::string>> LoadStringVector(
    const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    return absl::InvalidArgumentError(
        absl::StrCat("Error: Could not open ", path));
  }
  std::vector<std::string> strs;
  std::string line;
  while (std::getline(file, line)) {
    strs.push_back(line);
  }
  file.close();
  return strs;
}

}  // namespace

int main(int argc, const char *argv[]) {
  absl::StatusOr<std::vector<std::string>> board =
      LoadStringVector(absl::GetFlag(FLAGS_path_to_board_file));
  if (!board.ok()) {
    LOG(ERROR) << board.status();
    return 1;
  }

  absl::StatusOr<Solver> solver =
      Solver::CreateSolverWithSerializedTrie(*board);
  if (!solver.ok()) {
    LOG(ERROR) << solver.status();
    return 1;
  }

  if (absl::Status s = solver->SolveGreedily(); !s.ok()) {
    LOG(ERROR) << s;
    return 1;
  }

  LOG(INFO) << absl::StrCat("Greedy solution: \n", *solver);
  return 0;
}