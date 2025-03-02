#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/log/log.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "src/shared/dictionary_utils.h"
#include "src/spelltower/spelltower_board.h"
#include "src/spelltower/spelltower_solver.h"

using namespace puzzmo;
using ::spelltower::SpelltowerBoard;
using ::spelltower::SpelltowerSolver;

ABSL_FLAG(bool, run_regex, true, "Run regex mode instead of the DFS?");

int main(int argc, const char *argv[]) {
  // Read in the board
  std::vector<std::string> boardvec;
  std::ifstream boardfile("data/spelltower_board.txt");
  if (!boardfile.is_open()) {
    LOG(ERROR) << "Error: Could not open spelltower_board.txt";
    return 1;
  }
  std::string line;
  LetterCount letter_count;
  while (std::getline(boardfile, line)) {
    std::string row;
    std::istringstream iss(line);
    char c;
    while (iss >> c) {
      row.push_back(c);
      if (std::isalpha(c)) {
        auto s = letter_count.AddLetter(c);
      }
    }
    boardvec.push_back(row);
  }
  boardfile.close();
  SpelltowerBoard board(boardvec);

  // Read in the dictionary
  auto words = ReadDictionaryFileToVector(
      {.min_letters = 3, .max_letter_count = letter_count});
  if (!words.ok()) {
    LOG(ERROR) << words.status();
    return 1;
  }
  std::shared_ptr<TrieNode> dict = CreateDictionaryTrie(*words);

  // Create a spelltower and populate it with this data, then solve it
  SpelltowerSolver solver(board, dict);

  if (absl::GetFlag(FLAGS_run_regex)) {
    LOG(INFO) << solver.LongestPossibleAllStarWord();
    return 0;
  }

  auto results = solver.AllWordsOnBoard(board);
  LOG(INFO) << "All available words, by score:";
  for (const auto &[k, v] : results) {
    LOG(INFO) << absl::StrCat(k, ": ", absl::StrJoin(v, ", "));
  }

  return 0;
}