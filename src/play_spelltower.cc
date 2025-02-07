#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "absl/container/btree_map.h"
#include "absl/container/btree_set.h"
#include "absl/log/log.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "re2/re2.h"

#include "dictionary_utils.h"
#include "spelltower_board.h"
#include "spelltower_solver.h"

using namespace puzzmo;

int main(int argc, const char *argv[]) {
  // Read in the board
  std::vector<std::vector<char>> boardvec;
  std::ifstream boardfile("data/board_spelltower.txt");
  if (!boardfile.is_open()) {
    LOG(ERROR) << "Error: Could not open board_spelltower.txt";
    return 1;
  }
  std::string line;
  LetterCount letter_count;
  while (std::getline(boardfile, line)) {
    std::vector<char> row;
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
      {.min_letters = 3, .letter_count = letter_count});
  if (!words.ok()) {
    LOG(ERROR) << words.status();
    return 1;
  }

  if (false) {
    std::vector<std::string> filtered_words =
        board.MightHaveWords(*words,
                             /*all_star=*/true);
    LOG(INFO) << filtered_words.size();
    std::sort(filtered_words.begin(), filtered_words.end(),
              [](std::string a, std::string b) {
                if (a.length() == b.length())
                  return a < b;
                return a.length() < b.length();
              });

    LOG(INFO) << "All words in dictionary, shortest to longest:";
    std::string regexmonster =
        absl::StrJoin(board.GetAllStarRegexes(), "|",
                      [](std::string *out, const std::string &in) {
                        absl::StrAppend(out, "(", in, ")");
                      });
    for (const auto &w : filtered_words) {
      if (RE2::PartialMatch(w, regexmonster))
        LOG(INFO) << w;
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