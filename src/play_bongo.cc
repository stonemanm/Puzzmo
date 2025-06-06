#include <fstream>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/flags/flag.h"
#include "absl/log/log.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "bongo/dict.h"
#include "bongo/gamestate.h"
#include "bongo/solver.h"

//-------
// Files

ABSL_FLAG(std::string, path_to_board_file, "inputs/bongo_board.txt",
          "Input file containing a 5x5 char grid.");

ABSL_FLAG(std::string, path_to_tile_file, "inputs/bongo_tiles.txt",
          "Space-delimited input file where each line contains a letter, the "
          "number of that letter, and the value of that latter.");

//------------
// Parameters

ABSL_FLAG(
    int, tiles_for_bonus_words, 7,
    "The number to pass NMostValuableTiles, from which sets of 3 are chosen to "
    "make possible bonus words. Note that increasing this n scales by O(n^2).");

ABSL_FLAG(
    int, tiles_for_multiplier_tiles, 4,
    "The number to pass NMostValuableTiles, from which sets of 3 are chosen to "
    "make possible bonus words. Note that increasing this n scales by O(n^2).");

using namespace puzzmo;
using ::bongo::Dict;
using ::bongo::Gamestate;
using ::bongo::Solver;
using ::bongo::Technique;

using LettersToWordsMap =
    absl::flat_hash_map<LetterCount, absl::flat_hash_set<std::string>>;

namespace {

//---------
// Helpers

// Turns the lines of a file into strings in a vector.
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

// Loads data from the board and tile files and creates the starting state.
absl::StatusOr<Gamestate> LoadStartingState() {
  absl::StatusOr<std::vector<std::string>> grid_strings =
      LoadStringVector(absl::GetFlag(FLAGS_path_to_board_file));
  if (!grid_strings.ok()) return grid_strings.status();

  absl::StatusOr<std::vector<std::string>> tile_triples =
      LoadStringVector(absl::GetFlag(FLAGS_path_to_tile_file));
  if (!tile_triples.ok()) return tile_triples.status();

  absl::flat_hash_map<char, int> letter_values;
  LetterCount letters;
  for (const std::string &triple : *tile_triples) {
    std::vector<std::string> v = absl::StrSplit(triple, " ");
    if (v.size() != 3)
      return absl::InvalidArgumentError(absl::StrCat(
          "Error: line in tile file not properly formatted: ", triple));

    char letter = v[0][0];
    int count = std::stoi(v[1]);
    int value = std::stoi(v[2]);
    if (absl::Status s = letters.AddLetter(letter, count); !s.ok()) {
      LOG(ERROR) << s;
      return s;
    }
    letter_values[letter] = value;
  }
  return Gamestate(*grid_strings, letter_values, letters);
}

}  // namespace

// TODO: save output b/t runs to reduce duplicate work?
// "childof"
int main(int argc, const char *argv[]) {
  // Load the dictionary and the starting game state
  absl::StatusOr<Dict> dict = Dict::LoadFromFiles();
  if (!dict.ok()) {
    LOG(ERROR) << dict.status();
    return 1;
  }
  absl::StatusOr<Gamestate> starting_state = LoadStartingState();
  if (!starting_state.ok()) {
    LOG(ERROR) << starting_state.status();
    return 1;
  }
  if (starting_state->letters().size() < 25) {
    LOG(ERROR) << "Fewer than 25 letters provided in letter pool.";
    return 1;
  }

  // std::vector<Point> path;
  // int row = 4;
  // for (int col = 0; col < 5; ++col) {
  //   path.push_back({row, col});
  // }
  // absl::Status s = starting_state->FillLine(path, "trove");
  // if (!s.ok()) {
  //   LOG(ERROR) << s;
  //   return 1;
  // }
  // for (const Point &p : path) {
  //   (*starting_state)[p].is_locked = true;
  // }

  Solver bongo_solver(
      *dict, *starting_state,
      {.techniques = {Technique::kFillBonusWordCells},
       .num_tiles_for_bonus_words = absl::GetFlag(FLAGS_tiles_for_bonus_words),
       .num_tiles_for_mult_cells =
           absl::GetFlag(FLAGS_tiles_for_multiplier_tiles)});
  if (absl::StatusOr<Gamestate> solution = bongo_solver.Solve();
      !solution.ok()) {
    LOG(ERROR) << solution.status();
    return 1;
  } else {
    LOG(INFO) << *solution;
    return 0;
  }
}