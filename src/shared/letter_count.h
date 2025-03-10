#ifndef PUZZMO_SHARED_LETTERCOUNT_H_
#define PUZZMO_SHARED_LETTERCOUNT_H_

#include <string>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"

namespace puzzmo {

// A container for some quantity of (case-agnostic) letters.
class LetterCount {
 public:
  LetterCount() : counts_(26) {};
  explicit LetterCount(absl::string_view s);

  // Adds one or more copies of a given letter to the LetterCount, returning the
  // new count of that letter. AddLetter will return an error if c is not a
  // letter, or if i is not a positive integer. If an error is thrown,
  // LetterCount will be unchanged.
  absl::StatusOr<int> AddLetter(char c, int i);
  absl::StatusOr<int> AddLetter(char c) { return AddLetter(c, 1); }

  // Adds the letters in s to the LetterCount, ignoring all non-letter
  // characters in s.
  void AddLetters(absl::string_view s);

  // Removes one or more copies of a given letter to the LetterCount, returning
  // the new count of that letter. RemoveLetter will return an error if c is not
  // a letter, if i is not a positive integer, or if the LetterCount does not
  // contain as many copies of c as it attempts to remove. If an error is
  // thrown, LetterCount will be unchanged.
  absl::StatusOr<int> RemoveLetter(char c, int i);
  absl::StatusOr<int> RemoveLetter(char c) { return RemoveLetter(c, 1); }

  // Removes the letters in s from the LetterCount, ignoring all non-letter
  // characters in s. RemoveLetters will return an error if there are more
  // copies of a letter in s than in the LetterCount. If an error is thrown,
  // LetterCount will be unchanged.
  absl::Status RemoveLetters(absl::string_view s);

  // Returns an alphabetized string of the characters in this LetterCount.
  std::string CharsInOrder() const;

  // Returns a set of all k-letter combinations of the characters in this
  // LetterCount. Combinations are returned as alphabetized strings.
  absl::flat_hash_set<std::string> CombinationsOfSize(int k) const;

  // Returns regex that will match a single character iff that character is
  // contained within this LetterCount.
  std::string RegexMatchingContents() const;

  // Returns true if all characters in the parameter are contained within this
  // LetterCount.
  bool contains(const LetterCount &other) const;
  bool contains(char c) const { return count(c); }
  bool contains(absl::string_view other) const {
    return contains(LetterCount(other));
  }

  bool empty() const;
  int size() const;

  void set_count(char c, int i);
  int count(char c) const;

  LetterCount &operator+=(const LetterCount &rhs);
  LetterCount &operator+=(absl::string_view rhs) {
    return operator+=(LetterCount(rhs));
  }
  // To avoid the possibility of invalid states, LetterCount has a floor of 0
  // for any given value. To determine if no values in rhs are greater than
  // their counterparts in lhs, use Contains() rather than operator-=
  LetterCount &operator-=(const LetterCount &rhs);
  LetterCount &operator-=(absl::string_view rhs) {
    return operator-=(LetterCount(rhs));
  }

 private:
  // The underlying class data. The number of 'a's is stored at counts_[0], 'b's
  // at counts_[1], and so on.
  std::vector<int> counts_;

  // Allows hashing of LetterCount.
  template <typename H>
  friend H AbslHashValue(H h, const LetterCount &lc) {
    return H::combine(std::move(h), lc.counts_);
  }

  // Allows easy conversion of LetterCount to string.
  template <typename Sink>
  friend void AbslStringify(Sink &sink, const LetterCount &lc) {
    if (lc.empty()) return;
    std::vector<std::string> v;
    for (int i = 0; i < 26; ++i) {
      if (lc.counts_[i] == 0) continue;
      v.push_back(absl::StrCat(std::string(1, 'a' + i), ":", lc.counts_[i]));
    }
    sink.Append(absl::StrCat("[", absl::StrJoin(v, ", "), "]"));
  }
};

bool operator==(const LetterCount &lhs, const LetterCount &rhs);
bool operator!=(const LetterCount &lhs, const LetterCount &rhs);
LetterCount operator+(const LetterCount &lhs, const LetterCount &rhs);
LetterCount operator-(const LetterCount &lhs, const LetterCount &rhs);

}  // namespace puzzmo

#endif