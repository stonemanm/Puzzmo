#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "absl/container/btree_map.h"
#include "absl/container/btree_set.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "spelltower.h"

using namespace puzzmo;

int main(int argc, const char *argv[]) {
  // Read in the dictionary
  std::vector<std::string> words;
  std::string line;
  std::ifstream dictfile("data/dictionary.txt");
  if (!dictfile.is_open()) {
    LOG(ERROR) << "Error: Could not open dictionary.txt";
    return 1;
  }
  while (std::getline(dictfile, line)) {
    words.push_back(line);
  }
  dictfile.close();

  // Read in the board
  std::vector<std::vector<char>> board;
  std::ifstream boardfile("data/board_spelltower.txt");
  if (!boardfile.is_open()) {
    LOG(ERROR) << "Error: Could not open board_spelltower.txt";
    return 1;
  }
  while (std::getline(boardfile, line)) {
    std::vector<char> row;
    std::istringstream iss(line);
    char c;
    while (iss >> c) {
      row.push_back(c);
    }
    board.push_back(row);
  }
  boardfile.close();

  // Create a spelltower and populate it with this data, then solve it
  Spelltower spelltower(board);
  spelltower.AddToDictionary(words);
  WordMap results = spelltower.FindWords();
  LOG(INFO) << "All available words, by score:";
  for (const auto &[k, v] : results) {
    LOG(INFO) << absl::StrCat(k, ": ", absl::StrJoin(v, ", "));
  }

  return 0;
}