#include "solver.h"

#include <string>

#include "absl/container/flat_hash_set.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"

namespace puzzmo::bongo {

int Solver::CeilingForScore() const {
  auto values = starting_state_.values();
  std::string tiles_in_value_order = starting_state_.NMostValuableTiles(25);
  int score = 0;
  // 1 tile gets 3x score, 6 get 2x, and the rest get 1x.
  // Then multiply that all by 1.3.
  score += values[tiles_in_value_order[0]] * 3;
  for (int i = 1; i < 8; ++i) {
    score += values[tiles_in_value_order[i]] * 2;
  }
  for (int i = 8; i < tiles_in_value_order.size(); ++i) {
    score += values[tiles_in_value_order[i]];
  }

  return std::ceil(1.3 * score);
}

absl::StatusOr<Gamestate> Solver::FindSolutionWithScore(int score) const {
  return absl::UnimplementedError("To do");
}

absl::StatusOr<Gamestate> Solver::Solve() {
  std::vector<Point> bonus_path = starting_state_.bonus_path();
  std::vector<Point> multiplier_squares = starting_state_.MultiplierSquares();

  // // Get the triple multiplier square,,,
  // Point triple_square;
  // for (const Point &p : multiplier_squares) {
  //   if (starting_state_.multiplier_at(p) == 3) {
  //     triple_square = p;
  //     break;
  //   }
  // }

  // std::vector<Point> triple_path =
  // starting_state_.row_path(triple_square.row);

  // // ...and lock the highest-value tile on it...
  // std::string mvl = starting_state_.NMostValuableTiles(1);
  // BongoGameState bgs = starting_state_;
  // if (auto s = bgs.FillSquare(triple_square, mvl[0]); !s.ok()) return s;
  // bgs.set_is_locked_at(triple_square, true);

  // // ...then get the possible words for that row.
  // absl::flat_hash_set<std::string> triple_words_to_try =
  // dict_.GetMatchingWords(
  //     {.min_length = 3,
  //      .max_length = 5,
  //      .max_letters = starting_state_.letter_pool(),
  //      .matching_regex =  // bgs.RegexForPath(triple_path)
  //      ".*w$"});

  // int loops = 0;
  // for (const auto &triple_word : triple_words_to_try) {
  //   LOG(INFO) << "Beginning loop " << ++loops << " with bonus word \""
  //             << triple_word << "\"";

  //   // Place the triple word on the board
  //   BongoGameState current_board = starting_state_;
  //   if (auto s = current_board.FillPath(bonus_path, triple_word); !s.ok())
  //     return s;
  //   for (const Point &p : triple_path) {
  //     // curren
  //   }
  // }

  // To narrow the search space, grab the most valuable tiles and try to
  // make bonus words using three of them.
  absl::flat_hash_set<std::string> bonus_words_to_try;
  // If there are already letters in the bonus path, adjust this phase
  // accordingly
  LetterCount bplc(starting_state_.path_string(starting_state_.bonus_path()));
  LetterCount valuable_letters(
      starting_state_.NMostValuableTiles(tiles_for_bonus_words_));
  absl::flat_hash_set<std::string> combos =
      valuable_letters.CombinationsOfSize(3 - bplc.size());
  for (const auto &combo : combos) {
    auto words = dict_.GetMatchingWords(
        {.min_length = 4,
         .max_length = 4,
         .min_letters = LetterCount(combo),
         .max_letters = starting_state_.letter_pool() + bplc,
         .matching_regex = starting_state_.RegexForPath(bonus_path)});
    bonus_words_to_try.insert(words.begin(), words.end());
  }
  LOG(INFO) << "Found " << bonus_words_to_try.size() << " bonus words to try.";
  LOG(INFO) << absl::StrJoin(bonus_words_to_try, ", ");

  int loops = 0;
  for (const std::string &bonus_word : bonus_words_to_try) {
    LOG(INFO) << "Beginning loop " << ++loops << "/"
              << bonus_words_to_try.size() << " with bonus word \""
              << bonus_word << "\"";

    // Place the bonus word on the board
    Gamestate current_board = starting_state_;
    if (auto s = current_board.FillPath(bonus_path, bonus_word); !s.ok())
      return s;
    for (const Point &p : bonus_path) {
      current_board.set_is_locked_at(p, true);
    }

    // Grab the high-scoring tiles not used by the bonus word, and try all the
    // permutations of them on the open multiplier_squares.
    std::string top3 =
        current_board.NMostValuableTiles(tiles_for_multiplier_tiles_);
    std::sort(top3.begin(), top3.end());
    do {
      std::vector<Point> locked_here;
      for (int i = 0; i < 3; ++i) {
        Point p = multiplier_squares[i];
        if (std::isalpha(current_board.char_at(p))) continue;
        // If we have an error placing tiles, clean up the ones we have placed
        // so far and then move on.
        if (auto s = current_board.FillSquare(p, top3[i]); !s.ok()) return s;

        current_board.set_is_locked_at(p, true);
        locked_here.push_back(p);
      }

      if (auto s = FindWordsRecursively(current_board); !s.ok()) return s;

      for (int i = 0; i < locked_here.size(); ++i) {
        current_board.set_is_locked_at(locked_here[i], false);
        if (auto s = current_board.ClearSquare(locked_here[i]); !s.ok())
          return s;
      }
    } while (std::next_permutation(top3.begin(), top3.end()));
  }
  if (highest_score_ == 0) {
    ++tiles_for_bonus_words_;
    ++tiles_for_multiplier_tiles_;
    LOG(INFO) << "No solutions found. Trying again with a broader "
                 "search.";
    return Solve();
  }
  return highest_scoring_board_;
}

absl::Status Solver::FindWordsRecursively(Gamestate &current_board) {
  // If all rows are filled, score the board. Update highest_scoring_board_ if
  // appropriate
  // std::cout << "\r" << "[" << current_board.letter_board()[0] << "]" << "["
  //           << current_board.letter_board()[1] << "]" << "["
  //           << current_board.letter_board()[2] << "]" << "["
  //           << current_board.letter_board()[3] << "]" << "["
  //           << current_board.letter_board()[4] << "]";
  if (current_board.NumLetters() < 25) return absl::UnknownError("huh");
  if (current_board.IsComplete()) {
    if (int current_score = Score(current_board);
        current_score > highest_score_) {
      LOG(INFO) << absl::StrCat("New best score! (", current_score, ")");
      for (const auto &path : current_board.PathsToScore()) {
        std::string word = current_board.GetWord(path);
        LOG(INFO) << absl::StrCat(PathScore(current_board, path), " - ", word,
                                  (dict_.IsCommonWord(word) ? " is" : " isn't"),
                                  " a common word.");
      }

      LOG(INFO) << current_board;
      highest_scoring_board_ = current_board;
      highest_score_ = current_score;
    }
    return absl::OkStatus();
  }

  // Pick the most-filled row that has yet to be filled, and find all possible
  // matches.
  int row = current_board.MostRestrictedWordlessRow();
  absl::flat_hash_set<std::string> matches = dict_.GetMatchingWords(
      {.min_length = 5,
       .max_length = 5,
       .min_letters =
           LetterCount(current_board.path_string(current_board.row_path(row))),
       .max_letters =
           current_board.letter_pool() +
           LetterCount(current_board.path_string(current_board.row_path(row))),
       .matching_regex =
           current_board.RegexForPath(current_board.row_path(row))});

  // For each match, if it's possible to place it on the board, do so,
  // recurse, then backtrack.
  for (const auto &word : matches) {
    if (auto s = current_board.FillPath(current_board.row_path(row), word);
        !s.ok()) {
      return s;
    }

    // If something has gone wrong, throw that error all the way back
    // down.
    if (auto s = FindWordsRecursively(current_board); !s.ok()) {
      return s;
    }

    auto s = current_board.ClearPath(current_board.row_path(row));
    if (!s.ok()) return s;
  }

  return absl::OkStatus();
}

int Solver::Score(const Gamestate &bgs) const {
  int score = 0;
  for (const auto &path : bgs.PathsToScore()) {
    score += PathScore(bgs, path);
  }
  return score;
}

int Solver::PathScore(const Gamestate &bgs,
                      const std::vector<Point> &path) const {
  std::string word = bgs.GetWord(path);
  if (!dict_.IsValidWord(word)) return 0;

  // Find the index in path where word begins.
  int offset = 0;
  while (bgs.path_string(path).substr(offset, word.size()) != word) {
    ++offset;
  }

  int score = 0;
  for (int i = 0; i < word.size(); ++i) {
    char c = word[i];
    score += (bgs.values().at(c) * bgs.multiplier_at(path[i + offset]));
  }
  return std::ceil(score * (dict_.IsCommonWord(word) ? 1.3 : 1));
}

}  // namespace puzzmo::bongo