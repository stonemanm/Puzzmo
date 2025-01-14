#include <cctype>
#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "absl/log/log.h"
#include "absl/strings/str_join.h"

#include "bongo_solver.h"
#include "dictionary_utils.h"

using namespace puzzmo;

// Currently takes like an hour to run! Further optimization needed.
int main(int argc, const char *argv[]) {
  // Read in the tiles
  LetterCount tiles;
  std::ifstream tilefile("data/bongo_tiles.txt");
  if (!tilefile.is_open()) {
    LOG(ERROR) << "Error: Could not open bongo_tiles.txt";
    return 0;
  }
  std::string line;
  while (std::getline(tilefile, line)) {
    tiles += LetterCount(line);
  }
  tilefile.close();

  // Read in the dictionary
  absl::flat_hash_map<LetterCount, absl::flat_hash_set<std::string>> dict =
      CreateAnagramDictionary(
          ReadDictionaryFileToVector({.min_letters = 5,
                                      .max_letters = 5,
                                      .filter_by_letters = true,
                                      .letter_count = tiles}));

  // Create a solver with the loaded data, then solve
  BongoSolver bongo_solver(tiles, dict);

  absl::flat_hash_set<absl::flat_hash_set<std::string>> word_sets =
      bongo_solver.FindWordSets();
  LOG(ERROR) << "All available letter sets:";
  for (const auto &word_set : word_sets) {
    LOG(ERROR) << absl::StrJoin(word_set, ", ");
  }

  return 0;
}