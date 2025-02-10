#ifndef bongo_gamestate_h
#define bongo_gamestate_h

#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "bongo_dictionary.h"
#include "letter_count.h"
#include "point.h"

namespace puzzmo {

class BongoGameState {
 public:
  // board_strings should be a vector of length 5, containing 5-long strings
  BongoGameState(const std::vector<std::string> board_strings,
                 absl::flat_hash_map<char, int> letter_values,
                 LetterCount remaining_tiles,
                 std::vector<std::string> placed_tiles);
  BongoGameState(const std::vector<std::string> board_strings,
                 absl::flat_hash_map<char, int> letter_values,
                 LetterCount remaining_tiles)
      : BongoGameState(board_strings, letter_values, remaining_tiles,
                       std::vector<std::string>(5, std::string(5, '_'))) {};

  const absl::flat_hash_map<char, int> LetterValues() const {
    return letter_values_;
  }

  std::string NMostValuableTiles(int n) const;

  std::vector<std::string> PlacedTiles() const { return placed_tiles_; }
  LetterCount RemainingTiles() const { return remaining_tiles_; }

  bool Complete() const;
  std::vector<std::string> TilePlacements() const;

  bool PlaceTile(const Point &p, char c);
  bool FillRestOfWord(int row, absl::string_view word);
  bool RemoveTile(const Point &p);
  bool ClearRowExceptBonusTiles(int row);
  bool ClearBoardExceptBonusTiles();

  std::vector<Point> MultiplierSquares() const;
  std::vector<Point> BonusWordPath() const;

  std::string RowWord(int row) const;
  std::string BonusWord() const;

  // Calculates the score of the entire board. Uses the dictionary to check
  // validity and commonality of words.
  int BonusWordScore(const BongoDictionary &dict) const;
  int RowWordScore(int row, const BongoDictionary &dict) const;
  int Score(const BongoDictionary &dict) const;

  std::string RowRegex(int row) const;

  int IndexOfFullestIncompleteRow() const;

  friend bool operator==(const BongoGameState &lhs, const BongoGameState &rhs);
  friend bool operator<(const BongoGameState &lhs, const BongoGameState &rhs);

 private:
  std::vector<Point> bonus_word_path_;
  absl::flat_hash_map<char, int> letter_values_;
  std::vector<std::vector<int>> multipliers_;
  std::vector<std::string> placed_tiles_;
  LetterCount remaining_tiles_;

  bool HasPoint(const Point &p) const;

  template <typename H>
  friend H AbslHashValue(H h, const BongoGameState &bgs) {
    return H::combine(std::move(h), bgs.bonus_word_path_, bgs.letter_values_,
                      bgs.multipliers_, bgs.placed_tiles_,
                      bgs.remaining_tiles_);
  }

  template <typename Sink>
  friend void AbslStringify(Sink &sink, const BongoGameState &bgs) {
    absl::Format(&sink, "%v\n[%s]\n[%s]\n[%s]\n[%s]\n[%s]",
                 bgs.remaining_tiles_, bgs.placed_tiles_[0],
                 bgs.placed_tiles_[1], bgs.placed_tiles_[2],
                 bgs.placed_tiles_[3], bgs.placed_tiles_[4]);
  }
};

bool operator==(const BongoGameState &lhs, const BongoGameState &rhs);
bool operator<(const BongoGameState &lhs, const BongoGameState &rhs);

}  // namespace puzzmo

#endif