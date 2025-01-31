#ifndef bongo_gamestate_h
#define bongo_gamestate_h

#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"

#include "letter_count.h"
#include "point.h"

namespace puzzmo {

class BongoGameState {
public:
  // board_strings should be a vector of length 5, containing 5-long strings
  BongoGameState(const std::vector<std::string> board_strings,
                 const absl::flat_hash_map<char, int> letter_values,
                 LetterCount remaining_tiles,
                 std::vector<std::vector<char>> placed_tiles);
  BongoGameState(const std::vector<std::string> board_strings,
                 const absl::flat_hash_map<char, int> letter_values,
                 LetterCount remaining_tiles)
      : BongoGameState(board_strings, letter_values, remaining_tiles,
                       {{' ', ' ', ' ', ' ', ' '},
                        {' ', ' ', ' ', ' ', ' '},
                        {' ', ' ', ' ', ' ', ' '},
                        {' ', ' ', ' ', ' ', ' '},
                        {' ', ' ', ' ', ' ', ' '}}) {};

  const absl::flat_hash_map<char, int> LetterValues() const {
    return letter_values_;
  }
  std::vector<std::vector<char>> PlacedTiles() const { return placed_tiles_; }
  LetterCount RemainingTiles() const { return remaining_tiles_; }

  bool PlaceTile(const Point &p, char c);
  bool RemoveTile(const Point &p);

  std::string RowWord(int row) const;
  std::string BonusWord() const;

  // These do not check whether or not the word is legitimate!
  // They also cannot check whether the word gets the common-word 1.3x bonus.
  int RowWordScore(int i) const;
  int BonusWordScore() const;

private:
  std::vector<Point> bonus_word_path_;
  const absl::flat_hash_map<char, int> letter_values_;
  std::vector<std::vector<int>> multipliers_;
  std::vector<std::vector<char>> placed_tiles_;
  LetterCount remaining_tiles_;

  bool HasPoint(const Point &p) const;
};

} // namespace puzzmo

#endif