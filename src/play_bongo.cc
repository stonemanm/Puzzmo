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
#include "bongo/bongo_dictionary.h"
#include "bongo/bongo_gamestate.h"
#include "bongo/bongo_solver.h"

ABSL_FLAG(std::string, path_to_board_file, "data/bongo_board.txt",
          "Input file containing a 5x5 char grid.");

ABSL_FLAG(std::string, path_to_tile_file, "data/bongo_tiles.txt",
          "Space-delimited input file where each line contains a letter, the "
          "number of that letter, and the value of that latter.");

ABSL_FLAG(
    int, tiles_for_bonus_words, 7,
    "The number to pass NMostValuableTiles, from which sets of 3 are chosen to "
    "make possible bonus words. Note that increasing this n scales by O(n^2).");

ABSL_FLAG(
    int, tiles_for_multiplier_tiles, 4,
    "The number to pass NMostValuableTiles, from which sets of 3 are chosen to "
    "make possible bonus words. Note that increasing this n scales by O(n^2).");

using namespace puzzmo;
using ::bongo::BongoDictionary;
using ::bongo::BongoGameState;
using ::bongo::BongoSolver;

using LettersToWordsMap =
    absl::flat_hash_map<LetterCount, absl::flat_hash_set<std::string>>;

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

absl::StatusOr<BongoGameState> LoadStartingState() {
  auto board = LoadStringVector(absl::GetFlag(FLAGS_path_to_board_file));
  if (!board.ok()) return board.status();

  absl::flat_hash_map<char, int> letter_values;
  LetterCount letter_pool;
  auto tile_triples = LoadStringVector(absl::GetFlag(FLAGS_path_to_tile_file));
  if (!tile_triples.ok()) return tile_triples.status();

  for (const std::string &triple : *tile_triples) {
    std::vector<std::string> v = absl::StrSplit(triple, " ");
    if (v.size() != 3) {
      return absl::InvalidArgumentError(absl::StrCat(
          "Error: line in ", absl::GetFlag(FLAGS_path_to_tile_file),
          " not properly formatted: ", triple));
    }
    char c = v[0][0];
    auto s = letter_pool.AddLetter(c, std::stoi(v[1]));
    letter_values[c] = std::stoi(v[2]);
  }
  return BongoGameState(*board, letter_values, letter_pool);
}

}  // namespace

// TODO: save output b/t runs to reduce duplicate work?
// "childof"
int main(int argc, const char *argv[]) {
  // Load the dictionary and the starting game state
  BongoDictionary dict;
  if (absl::Status s = dict.Init(); !s.ok()) {
    LOG(ERROR) << s;
    return 1;
  }
  absl::StatusOr<BongoGameState> starting_state = LoadStartingState();
  if (!starting_state.ok()) {
    LOG(ERROR) << starting_state.status();
    return 1;
  }
  if (starting_state->NumLetters() < 25) {
    LOG(ERROR) << "Fewer than 25 letters provided in letter pool.";
    return 1;
  }

  std::vector<Point> path;
  int r = 4;
  for (int i = 1; i < 5; ++i) {
    path.push_back({r, i});
  }
  auto s = starting_state->FillPath(path, "show");
  if (!s.ok()) {
    LOG(ERROR) << s;
    return 1;
  }
  for (const auto &p : path) {
    starting_state->set_is_locked_at(p, true);
  }

  BongoSolver bongo_solver(
      dict, *starting_state,
      {.tiles_for_bonus_words = absl::GetFlag(FLAGS_tiles_for_bonus_words),
       .tiles_for_multiplier_tiles =
           absl::GetFlag(FLAGS_tiles_for_multiplier_tiles)});
  if (auto s = bongo_solver.Solve(); !s.ok()) {
    LOG(ERROR) << s.status();
    return 1;
  } else {
    LOG(INFO) << *s;
    return 0;
  }
}