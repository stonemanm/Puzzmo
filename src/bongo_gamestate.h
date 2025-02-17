#ifndef bongo_gamestate_h
#define bongo_gamestate_h

#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "bongo_dictionary.h"
#include "letter_count.h"
#include "point.h"

namespace puzzmo {

class BongoGameState {
 public:
  // board and letter_grid should be vector of length 5, containing 5-long
  // strings. If vectors or strings have length > 5, only the first 5 will be
  // saved. If vectors or strings have length < 5, empty spaces will fill what
  // remains.
  BongoGameState(const std::vector<std::string> board,
                 absl::flat_hash_map<char, int> letter_values,
                 LetterCount letters_remaining,
                 std::vector<std::string> letter_grid);
  BongoGameState(const std::vector<std::string> board,
                 absl::flat_hash_map<char, int> letter_values,
                 LetterCount letters_remaining)
      : BongoGameState(board, letter_values, letters_remaining, {}) {};

  // Places letter c at point p in the tile grid and updates letters_remaining_.
  // If another letter is already there, removes it and places c instead.
  // Returns an error if c is not in tiles_remaining_, p is not in the
  // gamestate, or p is locked.
  absl::Status PlaceLetter(const Point &p, char c);

  // Places the string at the points along the path, updating
  // letters_remaining_. Replaces any letters that are already in that row,
  // unless they are locked. Returns an error if word and path have different
  // lengths, if letters_remaining_ have insufficient characters to place the
  // word, or if a tile that needs to be overwritten is locked.
  absl::Status PlaceString(absl::string_view word,
                           const std::vector<Point> &path);

  // If a letter is at point p in the tile grid, removes it and updates
  // letters_remaining_. Returns an error if p is not in the gamespace or if p
  // is locked.
  absl::Status ClearLetter(const Point &p);

  // Removes all unlocked letters from the path. updating letters_remaining_.
  absl::Status ClearString(const std::vector<Point> &path);

  // Removes all unlocked letters from the board, updating letters_remaining_.
  absl::Status ClearBoard();

  // Grabs the longest consecutive substring of letters from the path. If it is
  // 3+ characters (or 4+ if it's the bonus path), return it; otherwise, returns
  // an empty string. Note that whether or not this is a dictionary word is not
  // validated.
  std::string GetWord(const std::vector<Point> &path) const;

  // Returns the index of the row with the most letters but doesn't have 3+
  // consecutive letters. Breaks ties in favor of the lowest index.
  int MostRestrictedWordlessRow() const;

  // Returns true iff every row has 3+ consecutive letters.
  bool Complete() const;

  // Returns a vector of the points that have tile multipliers on them.
  std::vector<Point> MultiplierSquares() const;

  // Returns the n remaining letters with the highest scores, with the highest
  // values first. If fewer than n letters remain, returns all of them.
  std::string NMostValuableTiles(int n) const;

  // Returns regex to match the path as it currently exists. Any letters will
  // match, and any nonalphabetical characters are replaced with regex matching
  // any of the letters remaining.
  std::string RegexForPath(const std::vector<Point> &path) const;

  // Uses the dictionary to check validity and commonality of words, then
  // calculates the relative score.
  int CalculateScore(const BongoDictionary &dict) const;
  int CalculatePathScore(const std::vector<Point> &path,
                         const BongoDictionary &dict) const;

  /** * * * * * * * * * * *
   * Accessors & mutators *
   * * * * * * * * * * * **/

  // Note: mutators allow setting chars not in the letter count, or overwriting
  // of locked points. Use with caution!

  void set_letters_remaining(LetterCount lc);
  LetterCount letters_remaining() const;

  void set_letter_grid(std::vector<std::string> grid);
  std::vector<std::string> letter_grid() const;
  void set_char_at(const Point &p, char c);
  void set_char_at(int row, int col, char c) { set_char_at({row, col}, c); }
  char char_at(const Point &p) const;
  char char_at(int row, int col) const { return char_at({row, col}); }

  void set_multiplier_grid(std::vector<std::vector<int>> grid);
  std::vector<std::vector<int>> multiplier_grid() const;
  void set_multiplier_at(const Point &p, int i);
  void set_multiplier_at(int row, int col, int i) {
    set_multiplier_at({row, col}, i);
  }
  int multiplier_at(const Point &p) const;
  int multiplier_at(int row, int col) const {
    return multiplier_at({row, col});
  }

  std::vector<Point> row_path(int row) const;
  void set_row_string(int row, absl::string_view sv);
  std::string row_string(int row) const;

  std::string path_string(const std::vector<Point> &path) const;

  void set_bonus_word_path(const std::vector<Point> &path);
  std::vector<Point> bonus_word_path() const;
  void set_bonus_string(absl::string_view sv);
  std::string bonus_string() const;

  void set_letter_values(absl::flat_hash_map<char, int> lvmap);
  absl::flat_hash_map<char, int> letter_values() const;

  void set_is_locked(std::vector<std::vector<bool>> is_locked);
  std::vector<std::vector<bool>> is_locked() const;

  void set_is_locked_at(const Point &p, bool is_locked);
  void set_is_locked_at(int row, int col, bool is_locked) {
    set_is_locked_at({row, col}, is_locked);
  }
  bool is_locked_at(const Point &p) const;
  bool is_locked_at(int row, int col) const { return is_locked_at({row, col}); }

 private:
  LetterCount letters_remaining_;
  std::vector<std::string> letter_grid_;
  std::vector<std::vector<int>> multiplier_grid_;
  std::vector<Point> bonus_word_path_;
  absl::flat_hash_map<char, int> letter_values_;
  std::vector<std::vector<bool>> is_locked_;

  /** * * * * * * * * *
   * Abseil functions *
   * * * * * * * * * **/

  // Allows hashing of BongoGameState.
  template <typename H>
  friend H AbslHashValue(H h, const BongoGameState &bgs) {
    return H::combine(std::move(h), bgs.letters_remaining_, bgs.letter_grid_,
                      bgs.multiplier_grid_, bgs.bonus_word_path_,
                      bgs.letter_values_, bgs.is_locked_);
  }

  // Allows easy conversion of BongoGameState to string.
  template <typename Sink>
  friend void AbslStringify(Sink &sink, const BongoGameState &bgs) {
    absl::Format(&sink, "%v\n[%s]\n[%s]\n[%s]\n[%s]\n[%s]",
                 bgs.letters_remaining_, bgs.letter_grid_[0],
                 bgs.letter_grid_[1], bgs.letter_grid_[2], bgs.letter_grid_[3],
                 bgs.letter_grid_[4]);
  }
};

bool operator==(const BongoGameState &lhs, const BongoGameState &rhs);
bool operator!=(const BongoGameState &lhs, const BongoGameState &rhs);

}  // namespace puzzmo

#endif