#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "absl/log/log.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_join.h"
#include "src/shared/dictionary_utils.h"

using namespace puzzmo;

using TypeshiftBoard = std::vector<absl::flat_hash_set<char>>;

namespace {

void DFS(std::shared_ptr<TrieNode> node, int i, const TypeshiftBoard &board,
         std::vector<std::string> &words) {
  if (node == nullptr) {
    return;
  }

  if (node->word != nullptr) {
    words.push_back(*node->word);
    return;  // All words are of the same length
  }

  for (const char c : board[i]) {
    std::shared_ptr<TrieNode> child = node->children[c - 'a'];
    DFS(child, i + 1, board, words);
  }
}

int UnusedLetters(const std::string &word, const TypeshiftBoard &board) {
  int new_letters = 0;
  for (int i = 0; i < board.size(); ++i) {
    if (board[i].contains(word[i])) {
      ++new_letters;
    }
  }
  return new_letters;
}

}  // namespace

int main(int argc, const char *argv[]) {
  // Read in the board
  TypeshiftBoard board;
  std::ifstream boardfile("inputs/typeshift_board.txt");
  if (!boardfile.is_open()) {
    LOG(ERROR) << "Error: Could not open typeshift_board.txt";
    return 1;
  }
  int word_length = 0;
  int total_letters = 0;
  std::string line;
  while (std::getline(boardfile, line)) {
    ++word_length;
    absl::flat_hash_set<char> row;
    std::istringstream iss(line);
    char c;
    while (iss >> c) {
      ++total_letters;
      row.insert(c);
    }
    board.push_back(row);
  }
  boardfile.close();

  // Read in the dictionary
  absl::StatusOr<std::vector<std::string>> words = ReadDictionaryFileToVector(
      {.min_letters = word_length, .max_letters = word_length});
  if (!words.ok()) {
    LOG(ERROR) << words.status();
    return 1;
  }
  std::shared_ptr<TrieNode> dict = CreateDictionaryTrie(*words);

  std::vector<std::string> answers;
  DFS(dict, 0, board, answers);

  absl::flat_hash_set<std::string> best_set;
  for (int i = 0; i < 20; ++i) {
    absl::flat_hash_set<std::string> temp_set;
    TypeshiftBoard temp_board = board;
    int temp_letters = total_letters;
    while (temp_letters > 0) {
      // Ensure that the first element in answers will use the most unused
      // letters.
      std::nth_element(answers.begin(), answers.begin(), answers.end(),
                       [temp_board](std::string a, std::string b) {
                         return UnusedLetters(a, temp_board) >
                                UnusedLetters(b, temp_board);
                       });
      temp_set.insert(answers[0]);
      temp_letters -= UnusedLetters(answers[0], temp_board);
      for (int i = 0; i < word_length; ++i) {
        temp_board[i].erase(answers[0][i]);
      }
    }
    if (best_set.empty() || temp_set.size() < best_set.size()) {
      best_set = temp_set;
    }
  }

  LOG(INFO) << absl::StrJoin(best_set, ", ");

  return 0;
}