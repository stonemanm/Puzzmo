#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/log/log.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "dictionary_utils.h"
#include "re2/re2.h"
#include "spelltower_board.h"
#include "spelltower_solver.h"

using namespace puzzmo;

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

  if (absl::GetFlag(FLAGS_run_regex)) {
    auto maybe_star_words = ReadDictionaryFileToVector(
        {.min_letters = 15,
         .min_letter_count = LetterCount(board.StarLetters())});
    if (!maybe_star_words.ok()) {
      LOG(ERROR) << maybe_star_words.status();
      return 1;
    }
    // std::sort(maybe_star_words->begin(), maybe_star_words->end(),
    //           [](std::string a, std::string b) {
    //             if (a.length() == b.length()) return a < b;
    //             return a.length() < b.length();
    //           });
    std::vector<std::string> filtered_words =
        board.MightHaveAllStarWords(*maybe_star_words);

    LOG(INFO) << "All words in dictionary, longest to shortest:";
    std::string regexmonster =
        absl::StrJoin(board.GetAllStarRegexes(), "|",
                      [](std::string *out, const std::string &in) {
                        absl::StrAppend(out, "(", in, ")");
                      });
    for (const auto &w : filtered_words) {
      if (RE2::PartialMatch(w, regexmonster)) LOG(INFO) << w;
    }
    return 0;
  }

  std::shared_ptr<TrieNode> dict = CreateDictionaryTrie(*words);

  // Create a spelltower and populate it with this data, then solve it
  SpelltowerSolver spelltower(board, dict);
  auto results = spelltower.FindWords();
  LOG(INFO) << "All available words, by score:";
  for (const auto &[k, v] : results) {
    LOG(INFO) << absl::StrCat(k, ": ", absl::StrJoin(v, ", "));
  }

  return 0;
}