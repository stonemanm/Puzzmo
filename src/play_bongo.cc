#include <algorithm>
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

#include "bongo_gamestate.h"
#include "dictionary_utils.h"

ABSL_FLAG(std::string, path_to_board_file, "data/bongo_board.txt",
          "Input file containing a 5x5 char grid.");

ABSL_FLAG(std::string, path_to_tile_file, "data/bongo_tiles.txt",
          "Space-delimited input file where each line contains a letter, the "
          "number of that letter, and the value of that latter.");

using namespace puzzmo;

using LettersToWordsMap =
    absl::flat_hash_map<LetterCount, absl::flat_hash_set<std::string>>;

namespace {

absl::StatusOr<std::vector<std::string>>
LoadStringVector(const std::string &path) {
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
  if (!board.ok())
    return board.status();

  absl::flat_hash_map<char, int> letter_values;
  LetterCount tiles;
  auto tile_triples = LoadStringVector(absl::GetFlag(FLAGS_path_to_tile_file));
  if (!tile_triples.ok())
    return tile_triples.status();

  for (const std::string &triple : *tile_triples) {
    std::vector<std::string> v = absl::StrSplit(triple, " ");
    if (v.size() != 3) {
      return absl::InvalidArgumentError(absl::StrCat(
          "Error: line in ", absl::GetFlag(FLAGS_path_to_tile_file),
          " not properly formatted: ", triple));
    }
    char c = v[0][0];
    auto s = tiles.AddLetter(c, std::stoi(v[1]));
    letter_values[c] = std::stoi(v[2]);
  }
  return BongoGameState(*board, letter_values, tiles);
}

absl::Status
FindWordsRecursively(const LettersToWordsMap &dict, BongoGameState &bgs,
                     absl::flat_hash_set<BongoGameState> &filled_boards) {
  // If all rows are filled, save the board state to some set.
  if (bgs.Complete()) {
    filled_boards.insert(bgs);
    return absl::OkStatus();
  }
  // For each row (starting with the most-filled), use the letter count and
  // regex to pull all potentially matching words
  int row = bgs.IndexOfFullestIncompleteRow();
  absl::flat_hash_set<std::string> matches = FindMatchesInAnagramDictionary(
      dict, bgs.RemainingTiles() + LetterCount(bgs.RowWord(row)),
      bgs.RowRegex(row));
  for (const auto &word : matches) {
    // If a word works, place it, recurse, then backtrack.
    if (!bgs.FillRestOfWord(row, word)) {
      return absl::UnknownError(absl::StrCat("Unable to call FillRestOfWord(",
                                             row, ", \"", word, "\").\n\n",
                                             bgs));
    }
    absl::Status s = FindWordsRecursively(dict, bgs, filled_boards);
    if (!s.ok()) {
      return s;
    }
    bgs.ClearRowExceptBonusTiles(row);
  }
  return absl::OkStatus();
}

} // namespace

// Currently takes like an hour to run! Further optimization needed.
int main(int argc, const char *argv[]) {
  // Load bongo-specific data from the input files.
  auto status_or_bgs = LoadStartingState();
  if (!status_or_bgs.ok()) {
    LOG(ERROR) << status_or_bgs.status();
    return 1;
  }
  BongoGameState starting_state = *status_or_bgs;

  // Get all possible five-letter words with the given letters.
  auto words = ReadDictionaryFileToVector(
      {.min_letters = 5,
       .max_letters = 5,
       .letter_count = starting_state.RemainingTiles()});
  if (!words.ok()) {
    LOG(ERROR) << words.status();
    return 1;
  }

  // Get all possible four-letter words with the given letters.
  auto bonus_words = ReadDictionaryFileToVector(
      {.min_letters = 4,
       .max_letters = 4,
       .letter_count = starting_state.RemainingTiles()});
  if (!bonus_words.ok()) {
    LOG(ERROR) << bonus_words.status();
    return 1;
  }

  LettersToWordsMap dict = CreateAnagramDictionary(*words);
  LettersToWordsMap bonus_dict = CreateAnagramDictionary(*bonus_words);

  LetterCount valuable_letters(starting_state.NMostValuableTiles(7));
  absl::flat_hash_set<std::string> combos =
      valuable_letters.CombinationsOfSize(3);

  absl::flat_hash_set<std::string> filtered_bonus_words;
  for (const auto &combo : combos) {
    auto words = FindMatchesInAnagramDictionary(bonus_dict, LetterCount(combo));
    filtered_bonus_words.insert(words.begin(), words.end());
  }

  std::vector<std::string> fbwvec(filtered_bonus_words.begin(),
                                  filtered_bonus_words.end());
  std::sort(fbwvec.begin(), fbwvec.end(),
            [starting_state](std::string &lhs, std::string &rhs) {
              int l = starting_state.WordScore(lhs);
              int r = starting_state.WordScore(rhs);
              return l == r ? lhs < rhs : l > r;
            });

  absl::flat_hash_set<BongoGameState> filled_boards;
  // For each high-value bonus word!
  for (const auto &bonus_word : filtered_bonus_words) {
    // - Create a board state with the word in place
    BongoGameState bgs = starting_state;
    for (int i = 0; i < 4; ++i) {
      bgs.PlaceTile(starting_state.BonusWordPath()[i], bonus_word[i]);
    }

    // - Get the high-scoring tiles *not* used in the bonus word
    std::string top3 = bgs.NMostValuableTiles(4);
    std::sort(top3.begin(), top3.end());

    // - Place them on the bonus tiles (try all permutations, ideally)
    // - For each permutation: (24 per, I believe)
    std::vector<Point> multiplier_squares = starting_state.MultiplierSquares();
    do {
      bool skip = false;
      for (int i = 0; i < 3; ++i) {
        // If we have an error placing tiles, clean up the ones we have placed
        // so far and then move on.
        if (!bgs.PlaceTile(multiplier_squares[i], top3[i])) {
          skip = true;
          LOG(ERROR) << "Error placing " << top3[i] << " at point "
                     << multiplier_squares[i];
          for (int j = 0; j < i; ++j) {
            bgs.RemoveTile(multiplier_squares[j]);
          }
          break;
        }
      }
      if (skip)
        continue;

      absl::Status s = FindWordsRecursively(dict, bgs, filled_boards);
      if (!s.ok()) {
        LOG(ERROR) << s;
        return 1;
      }

      for (int i = 0; i < 3; ++i) {
        bgs.RemoveTile(multiplier_squares[i]);
      }
    } while (std::next_permutation(top3.begin(), top3.end()));
  }
  // For each potential board state:
  // - Calculate the score?
  // - Throw out below the 10 best?
  std::vector<BongoGameState> filled_boards_vec(filled_boards.begin(),
                                                filled_boards.end());
  std::partial_sort(filled_boards_vec.begin(), filled_boards_vec.begin() + 10,
                    filled_boards_vec.end(),
                    [starting_state](BongoGameState &lhs, BongoGameState &rhs) {
                      return lhs.Score() > rhs.Score();
                    });

  for (int i = 0; i < 10; ++i) {
    if (i < filled_boards_vec.size())
      LOG(ERROR) << filled_boards_vec[i];
  }

  // Pass the dictionary and the bgs to a BongoSolver, then solve.
  // BongoSolver bongo_solver(starting_state, *words);

  // absl::flat_hash_set<absl::flat_hash_set<std::string>> word_sets =
  //     bongo_solver.FindWordSets();
  // LOG(ERROR) << "All available letter sets:";
  // for (const auto &word_set : word_sets) {
  //   LOG(ERROR) << absl::StrJoin(word_set, ", ");
  // }

  return 0;
}