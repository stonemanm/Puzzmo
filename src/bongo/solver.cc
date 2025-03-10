#include "solver.h"

#include <string>

#include "absl/container/flat_hash_set.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"

namespace puzzmo::bongo {

// Constructors

Solver::Solver(const Dict &dict, const Gamestate &state, Parameters params)
    : dict_(dict),
      starting_state_(state),
      state_(state),
      best_state_(state),
      tiles_for_bonus_words_(params.tiles_for_bonus_words),
      tiles_for_multiplier_tiles_(params.tiles_for_multiplier_tiles) {}

int Solver::CeilingForScore() const {
  auto values = starting_state_.letter_values();
  std::string tiles_in_value_order = starting_state_.NMostValuableLetters(25);
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

// Mutators

void Solver::reset() { state_ = starting_state_; }

absl::StatusOr<Gamestate> Solver::Solve() {
  std::vector<Point> bonus_path = starting_state_.bonus_line();
  std::vector<Point> multiplier_squares = starting_state_.MultiplierCells();

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
  LetterCount bplc(starting_state_.LineString(starting_state_.bonus_line()));
  LetterCount valuable_letters(
      starting_state_.NMostValuableLetters(tiles_for_bonus_words_));
  absl::flat_hash_set<std::string> combos =
      valuable_letters.CombinationsOfSize(3 - bplc.size());
  for (const auto &combo : combos) {
    auto words = dict_.GetMatchingWords(
        {.min_length = 4,
         .max_length = 4,
         .min_letters = LetterCount(combo),
         .max_letters = starting_state_.unplaced_letters() + bplc,
         .matching_regex = starting_state_.LineRegex(bonus_path)});
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
    if (auto s = current_board.FillLine(bonus_path, bonus_word); !s.ok())
      return s;
    for (const Point &p : bonus_path) {
      current_board[p].is_locked = true;
    }

    // Grab the high-scoring tiles not used by the bonus word, and try all the
    // permutations of them on the open multiplier_squares.
    std::string top3 =
        current_board.NMostValuableLetters(tiles_for_multiplier_tiles_);
    std::sort(top3.begin(), top3.end());
    do {
      std::vector<Point> locked_here;
      for (int i = 0; i < 3; ++i) {
        const Point p = multiplier_squares[i];
        Cell &cell = current_board[p];
        if (std::isalpha(cell.letter)) continue;
        // If we have an error placing tiles, clean up the ones we have placed
        // so far and then move on.
        if (absl::Status s = current_board.FillCell(p, top3[i]); !s.ok())
          return s;

        cell.is_locked = true;
        locked_here.push_back(p);
      }

      if (absl::Status s = FindWordsRecursively(current_board); !s.ok())
        return s;

      for (int i = 0; i < locked_here.size(); ++i) {
        current_board[locked_here[i]].is_locked = false;
        if (absl::Status s = current_board.ClearCell(locked_here[i]); !s.ok())
          return s;
      }
    } while (std::next_permutation(top3.begin(), top3.end()));
  }
  if (best_score_ == 0) {
    ++tiles_for_bonus_words_;
    ++tiles_for_multiplier_tiles_;
    LOG(INFO) << "No solutions found. Trying again with a broader "
                 "search.";
    return Solve();
  }
  return best_state_;
}

absl::Status Solver::FindWordsRecursively(Gamestate &current_board) {
  // If all rows are filled, score the board. Update highest_scoring_board_ if
  // appropriate
  // std::cout << "\r" << "[" << current_board.letter_board()[0] << "]" << "["
  //           << current_board.letter_board()[1] << "]" << "["
  //           << current_board.letter_board()[2] << "]" << "["
  //           << current_board.letter_board()[3] << "]" << "["
  //           << current_board.letter_board()[4] << "]";
  if (current_board.letters().size() < 25) return absl::UnknownError("huh");
  if (current_board.IsComplete()) {
    if (int current_score = Score(current_board); current_score > best_score_) {
      LOG(INFO) << absl::StrCat("New best score! (", current_score, ")");
      for (const auto &path : current_board.LinesToScore()) {
        std::string word = current_board.GetWord(path);
        LOG(INFO) << absl::StrCat(LineScore(current_board, path), " - ", word,
                                  (dict_.IsCommonWord(word) ? " is" : " isn't"),
                                  " a common word.");
      }

      LOG(INFO) << current_board;
      best_state_ = current_board;
      best_score_ = current_score;
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
           LetterCount(current_board.LineString(current_board.line(row))),
       .max_letters =
           current_board.unplaced_letters() +
           LetterCount(current_board.LineString(current_board.line(row))),
       .matching_regex = current_board.LineRegex(current_board.line(row))});

  // For each match, if it's possible to place it on the board, do so,
  // recurse, then backtrack.
  for (const auto &word : matches) {
    if (absl::Status s = current_board.FillLine(current_board.line(row), word);
        !s.ok()) {
      return s;
    }

    // If something has gone wrong, throw that error all the way back
    // down.
    if (auto s = FindWordsRecursively(current_board); !s.ok()) {
      return s;
    }

    auto s = current_board.ClearLine(current_board.line(row));
    if (!s.ok()) return s;
  }

  return absl::OkStatus();
}

// Score

int Solver::LineScore(const Gamestate &bgs,
                      const std::vector<Point> &line) const {
  std::string word = bgs.GetWord(line);
  if (!dict_.IsValidWord(word)) return 0;

  // Find the index in path where word begins.
  int offset = 0;
  while (bgs.LineString(line).substr(offset, word.size()) != word) {
    ++offset;
  }

  int score = 0;
  for (int i = 0; i < word.size(); ++i) {
    char c = word[i];
    score += bgs.letter_values().at(c) * bgs[line[i + offset]].multiplier;
  }
  return std::ceil(score * (dict_.IsCommonWord(word) ? 1.3 : 1));
}

int Solver::Score(const Gamestate &bgs) const {
  int score = 0;
  for (const auto &line : bgs.LinesToScore()) {
    score += LineScore(bgs, line);
  }
  return score;
}

}  // namespace puzzmo::bongo