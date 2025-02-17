#include "bongo_solver.h"

#include <string>

#include "absl/container/flat_hash_set.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"

namespace puzzmo {

absl::StatusOr<BongoGameState> BongoSolver::Solve() {
  std::vector<Point> bonus_word_squares = starting_state_.bonus_word_path();
  std::vector<Point> multiplier_squares = starting_state_.MultiplierSquares();

  // Point triple_square;
  // for (const Point &p : multiplier_squares) {
  //   if (starting_state_.multiplier_at(p) == 3) {
  //     triple_square = p;
  //     break;
  //   }
  // }

  // std::string mvl = starting_state_.NMostValuableTiles(1);
  // BongoGameState current_board = starting_state_;
  // if (auto s = current_board.PlaceLetter(triple_square, mvl[0]); !s.ok())
  //   return s;

  // absl::flat_hash_set<std::string> triple_words_to_try =
  // dict_.GetMatchingWords(
  //     {.min_length = 5,
  //      .max_length = 5,
  //      .max_letters = starting_state_.letters_remaining(),
  //      .matching_regex = current_board.RegexForRow(triple_square.row)});

  // To narrow the search space, grab the most valuable tiles and try to
  // make bonus words using three of them.
  absl::flat_hash_set<std::string> bonus_words_to_try;
  LetterCount valuable_letters(
      starting_state_.NMostValuableTiles(tiles_for_bonus_words_));
  absl::flat_hash_set<std::string> combos =
      valuable_letters.CombinationsOfSize(3);
  for (const auto &combo : combos) {
    auto words = dict_.GetMatchingWords(
        {.min_length = 4,
         .max_length = 4,
         .min_letters = LetterCount(combo),
         .max_letters = starting_state_.letters_remaining(),
         .matching_regex = starting_state_.RegexForBonus()});
    bonus_words_to_try.insert(words.begin(), words.end());
  }
  LOG(INFO) << "Found " << bonus_words_to_try.size() << " bonus words to try.";
  LOG(INFO) << absl::StrJoin(bonus_words_to_try, ", ");

  int loops = 0;
  for (const std::string &bonus_word : bonus_words_to_try) {
    LOG(INFO) << "Beginning loop " << ++loops << " with bonus word \""
              << bonus_word << "\"";

    // Place the bonus word on the board
    BongoGameState current_board = starting_state_;
    if (auto s = current_board.PlaceBonusWord(bonus_word); !s.ok()) return s;
    for (const Point &p : bonus_word_squares) {
      current_board.set_is_locked_at(p, true);
    }

    // Grab the high-scoring tiles not used by the bonus word, and try all the
    // permutations of them on the open multiplier_squares.
    std::string top3 =
        current_board.NMostValuableTiles(tiles_for_multiplier_tiles_);
    std::sort(top3.begin(), top3.end());
    do {
      bool skip = false;

      std::vector<Point> locked_here;
      for (int i = 0; i < 3; ++i) {
        if (std::isalpha(current_board.char_at(multiplier_squares[i])))
          continue;
        // If we have an error placing tiles, clean up the ones we have placed
        // so far and then move on.
        if (auto s = current_board.PlaceLetter(multiplier_squares[i], top3[i]);
            !s.ok())
          return s;

        current_board.set_is_locked_at(multiplier_squares[i], true);
        locked_here.push_back(multiplier_squares[i]);
      }

      if (auto s = FindWordsRecursively(current_board); !s.ok()) return s;

      for (int i = 0; i < locked_here.size(); ++i) {
        current_board.set_is_locked_at(locked_here[i], false);
        if (auto s = current_board.RemoveLetter(locked_here[i]); !s.ok())
          return s;
      }
    } while (std::next_permutation(top3.begin(), top3.end()));
  }
  return highest_scoring_board_;
}

absl::Status BongoSolver::FindWordsRecursively(BongoGameState &current_board) {
  // If all rows are filled, score the board. Update highest_scoring_board_ if
  // appropriate
  if (current_board.Complete()) {
    if (current_board.CalculateScore(dict_) >
        highest_scoring_board_.CalculateScore(dict_)) {
      LOG(INFO) << absl::StrCat("New best score! (",
                                current_board.CalculateScore(dict_), ")");
      for (int i = 0; i < 5; ++i) {
        LOG(INFO) << absl::StrCat(
            current_board.CalculateRowScore(i, dict_), " - ",
            current_board.RowWord(i),
            (dict_.IsCommonWord(current_board.RowWord(i)) ? " is" : " isn't"),
            " a common word.");
      }
      LOG(INFO) << absl::StrCat(
          current_board.CalculateBonusScore(dict_), " - ",
          current_board.BonusWord(),
          (dict_.IsCommonWord(current_board.BonusWord()) ? " is" : " isn't"),
          " a common word.");
      LOG(INFO) << current_board;
      highest_scoring_board_ = current_board;
    }
    return absl::OkStatus();
  }

  // Pick the most-filled row that has yet to be filled, and find all possible
  // matches.
  int row = current_board.MostRestrictedWordlessRow();
  absl::flat_hash_set<std::string> matches = dict_.GetMatchingWords(
      {.min_length = 5,
       .max_length = 5,
       .min_letters = LetterCount(current_board.row_string(row)),
       .max_letters = current_board.letters_remaining() +
                      LetterCount(current_board.row_string(row)),
       .matching_regex = current_board.RegexForRow(row)});

  // For each match, if it's possible to place it on the board, do so, recurse,
  // then backtrack.
  for (const auto &word : matches) {
    if (auto s = current_board.PlaceWord(word, row); !s.ok()) {
      return s;
    }

    // If something has gone wrong, throw that error all the way back
    // down.
    if (auto s = FindWordsRecursively(current_board); !s.ok()) {
      return s;
    }

    auto s = current_board.RemoveWord(row);
    if (!s.ok()) return s;
  }

  return absl::OkStatus();
}

}  // namespace puzzmo