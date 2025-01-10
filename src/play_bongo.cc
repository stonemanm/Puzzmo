#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "absl/log/log.h"
#include "absl/strings/str_join.h"
#include "bongo_solver.h"
#include "dictionary_utils.h"

using namespace puzzmo;

int main(int argc, const char *argv[]) {
  // Read in the tiles
  std::vector<int> letters;
  std::ifstream tilefile("data/bongo_tiles.txt");
  if (!tilefile.is_open()) {
    LOG(ERROR) << "Error: Could not open bongo_tiles.txt";
    return 0;
  }
  std::string line;
  while (std::getline(tilefile, line)) {
    for (char c : line) {
      if (!islower(c))
        continue;
      ++letters[c - 'a'];
    }
  }
  tilefile.close();

  // Read in the dictionary
  std::vector<std::string> words =
      ReadDictionaryFileToVector({.min_letters = 5,
                                  .max_letters = 5,
                                  .filter_by_letters = true,
                                  .letter_counts = letters});
  absl::flat_hash_set<std::string> dict(words.begin(), words.end());

  // Create a solver with the loaded data, then solve
  BongoSolver bongo_solver(dict, letters);

  absl::flat_hash_set<absl::flat_hash_set<std::string>> word_sets =
      bongo_solver.FindWordSets();
  LOG(INFO) << "All available word sets:";
  for (const auto &word_set : word_sets) {
    LOG(INFO) << absl::StrJoin(word_set, ", ");
  }

  return 0;
}