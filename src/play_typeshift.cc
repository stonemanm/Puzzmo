#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "absl/log/log.h"
#include "absl/strings/str_join.h"

#include "dictionary_utils.h"

using namespace puzzmo;

using TypeshiftBoard = std::vector<absl::flat_hash_set<char>>;

namespace {

void DFS(std::shared_ptr<TrieNode> node, int i, const TypeshiftBoard &board,
         std::vector<std::string> &words) {
  if (i > 5 || node == nullptr) {
    return;
  }

  if (node->word != nullptr) {
    words.push_back(*node->word);
    return; // All words are of the same length
  }

  for (const char c : board[i]) {
    std::shared_ptr<TrieNode> child = node->children[c - 'a'];
    DFS(child, i + 1, board, words);
  }
}

} // namespace

int main(int argc, const char *argv[]) {
  // Read in the dictionary
  std::vector<std::string> words =
      ReadDictionaryFileToVector({.min_letters = 5, .max_letters = 5});
  std::shared_ptr<TrieNode> dict = CreateDictionaryTrie(words);

  // Read in the board
  TypeshiftBoard board;
  std::ifstream boardfile("data/board_typeshift.txt");
  if (!boardfile.is_open()) {
    LOG(ERROR) << "Error: Could not open board_typeshift.txt";
    return 1;
  }
  std::string line;
  while (std::getline(boardfile, line)) {
    absl::flat_hash_set<char> row;
    std::istringstream iss(line);
    char c;
    while (iss >> c) {
      row.insert(c);
    }
    board.push_back(row);
  }
  boardfile.close();

  std::vector<std::string> answers;
  DFS(dict, 0, board, answers);
  std::sort(answers.begin(), answers.end());
  LOG(INFO) << absl::StrJoin(answers, ", ");

  return 0;
}