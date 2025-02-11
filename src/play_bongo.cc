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
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "bongo_dictionary.h"
#include "bongo_gamestate.h"

ABSL_FLAG(std::string, path_to_board_file, "data/bongo_board.txt",
          "Input file containing a 5x5 char grid.");

ABSL_FLAG(std::string, path_to_tile_file, "data/bongo_tiles.txt",
          "Space-delimited input file where each line contains a letter, the "
          "number of that letter, and the value of that latter.");

ABSL_FLAG(
    int, tiles_for_bonus_words, 8,
    "The number to pass NMostValuableTiles, from which sets of 3 are chosen to "
    "make possible bonus words. Note that increasing this n scales by O(n^2).");

ABSL_FLAG(
    int, tiles_for_multiplier_tiles, 8,
    "The number to pass NMostValuableTiles, from which sets of 3 are chosen to "
    "make possible bonus words. Note that increasing this n scales by O(n^2).");

using namespace puzzmo;

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
  LetterCount tiles;
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
    auto s = tiles.AddLetter(c, std::stoi(v[1]));
    letter_values[c] = std::stoi(v[2]);
  }
  return BongoGameState(*board, letter_values, tiles);
}

absl::Status FindWordsRecursively(const BongoDictionary &dict,
                                  BongoGameState &current_board,
                                  BongoGameState &best_so_far) {
  // If all rows are filled, score the board. Update best_so_far if appropriate
  if (current_board.Complete()) {
    if (current_board.Score(dict) > best_so_far.Score(dict)) {
      LOG(INFO) << "New best score! (" << current_board.Score(dict) << ")";
      for (int i = 0; i < 5; ++i) {
        LOG(INFO) << current_board.RowWordScore(i, dict) << " - "
                  << current_board.RowWord(i)
                  << (dict.IsCommonWord(current_board.RowWord(i)) ? " is "
                                                                  : " isn't ")
                  << "a common word.";
      }
      LOG(INFO) << current_board.BonusWordScore(dict) << " - "
                << current_board.BonusWord()
                << (dict.IsCommonWord(current_board.BonusWord()) ? " is "
                                                                 : " isn't ")
                << "a common word.";
      LOG(INFO) << current_board;
      best_so_far = current_board;
    }
    return absl::OkStatus();
  }

  // Pick the most-filled row that has yet to be filled, and find all possible
  // matches.
  int row = current_board.IndexOfFullestIncompleteRow();
  absl::flat_hash_set<std::string> matches = dict.GetMatchingWords(
      {.min_length = 5,
       .max_length = 5,
       .min_letters = LetterCount(current_board.RowWord(row)),
       .max_letters = current_board.RemainingTiles() +
                      LetterCount(current_board.RowWord(row)),
       .matching_regex = current_board.RowRegex(row)});

  // For each match, if it's possible to place it on the board, do so, recurse,
  // then backtrack.
  for (const auto &word : matches) {
    if (!current_board.FillRestOfWord(row, word)) {
      return absl::UnknownError(absl::StrCat("Unable to call FillRestOfWord(",
                                             row, ", \"", word, "\").\n\n",
                                             current_board));
    }

    // If something has gone wrong, throw that error all the way back down.
    if (absl::Status s = FindWordsRecursively(dict, current_board, best_so_far);
        !s.ok()) {
      return s;
    }
    current_board.ClearRowExceptBonusTiles(row);
  }

  return absl::OkStatus();
}

}  // namespace

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

  // To narrow the search space, grab the most valuable tiles and try to
  // make bonus words using three of them.
  absl::flat_hash_set<std::string> bonus_words_to_try;
  LetterCount valuable_letters(starting_state->NMostValuableTiles(
      absl::GetFlag(FLAGS_tiles_for_bonus_words)));
  absl::flat_hash_set<std::string> combos =
      valuable_letters.CombinationsOfSize(3);
  for (const auto &combo : combos) {
    auto words = dict.GetMatchingWords(
        {.min_length = 4,
         .max_length = 4,
         .min_letters = LetterCount(combo),
         .max_letters = starting_state->RemainingTiles()});
    bonus_words_to_try.insert(words.begin(), words.end());
  }
  LOG(INFO) << "Found " << bonus_words_to_try.size() << " bonus words to try.";
  LOG(INFO) << absl::StrJoin(bonus_words_to_try, ", ");

  std::vector<Point> bonus_word_squares = starting_state->BonusWordPath();
  std::vector<Point> multiplier_squares = starting_state->MultiplierSquares();
  BongoGameState top_scorer = *starting_state;
  int loops = 0;
  for (const std::string &bonus_word : bonus_words_to_try) {
    LOG(ERROR) << "Beginning loop " << ++loops << " with bonus word \""
               << bonus_word << "\"";

    // Place the bonus word on the board
    BongoGameState current_board = *starting_state;
    for (int i = 0; i < bonus_word.size(); ++i) {
      current_board.PlaceTile(bonus_word_squares[i], bonus_word[i]);
    }

    // Grab the high-scoring tiles not used by the bonus word, and try all the
    // permutations of them on the multiplier_squares.
    std::string top3 = current_board.NMostValuableTiles(
        absl::GetFlag(FLAGS_tiles_for_multiplier_tiles));
    std::sort(top3.begin(), top3.end());
    do {
      bool skip = false;
      for (int i = 0; i < 3; ++i) {
        // If we have an error placing tiles, clean up the ones we have placed
        // so far and then move on.
        if (!current_board.PlaceTile(multiplier_squares[i], top3[i])) {
          skip = true;
          LOG(ERROR) << "Error placing " << top3[i] << " at point "
                     << multiplier_squares[i];
          for (int j = 0; j < i; ++j) {
            current_board.RemoveTile(multiplier_squares[j]);
          }
          break;
        }
      }
      if (skip) continue;

      if (absl::Status s =
              FindWordsRecursively(dict, current_board, top_scorer);
          !s.ok()) {
        LOG(ERROR) << s;
        return 1;
      }

      for (int i = 0; i < multiplier_squares.size(); ++i) {
        current_board.RemoveTile(multiplier_squares[i]);
      }
    } while (std::next_permutation(top3.begin(), top3.end()));
  }

  LOG(INFO) << top_scorer;
  return 0;
}