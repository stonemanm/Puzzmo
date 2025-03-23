// -----------------------------------------------------------------------------
// File: letter_count.h
// -----------------------------------------------------------------------------
//
// This header file defines a class that holds a count for each unique letter in
// en-US. It can be used to determine whether a string contains some set of
// letters, among other things.

#ifndef PUZZMO_SHARED_LETTERCOUNT_H_
#define PUZZMO_SHARED_LETTERCOUNT_H_

#include <string>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"

namespace puzzmo {

// puzzmo::LetterCount
//
// The `LetterCount` class contains a non-negative integer corresponding to
// every lowercase letter in en-US. Input sanitization casts uppercase letters
// to their lowercase counterparts and either throws an error or discards
// non-letters that are passed in.
class LetterCount {
 public:
  //--------------
  // Constructors

  // An empty `LetterCount` will have a count of 0 for each letter.
  LetterCount() : counts_(26) {};

  // When a `LetterCount` is created from a string of chars, all uppercase
  // letters in the string are first cast to lowercase, and all non-letters in
  // the string will be ignored.
  explicit LetterCount(const absl::string_view s);

  //-----------
  // Accessors

  // LetterCount::counts()
  //
  // Returns a vector in which index `c - 'a'` contains the number of `c`s
  // contained in this `LetterCount`.
  std::vector<int> counts() const { return counts_; }

  // LetterCount::operator[]
  //
  // The overloaded subscript operator returns the quantity of char `c`
  // contained. If `c` is not a lowercase letter, exhibits undefined behavior.
  int &operator[](char c) { return counts_[c - 'a']; }
  const int &operator[](char c) const { return counts_[c - 'a']; }

  // LetterCount::count()
  //
  // A version of `operator[]` without undefined behavior. If `c` is a letter,
  // returns the number of copies of it. If `c` is not a letter, returns `0`.
  int count(char c) const;

  // LetterCount::contains()
  //
  // Returns `true` if the quantity of each letter in `other` is less than or
  // equal to the number of it contained in this `LetterCount`.
  bool contains(const LetterCount &other) const;
  bool contains(const absl::string_view other) const {
    return contains(LetterCount(other));
  }
  bool contains(char other) const { return count(other); }

  // LetterCount::empty()
  //
  // Returns `true` if no letters are contained.
  bool empty() const;

  // LetterCount::size()
  //
  // Returns the sum of the counts of every letter.
  int size() const;

  // LetterCount::CharsInOrder()
  //
  // Returns an alphabetized string of the letters in this `LetterCount`.
  std::string CharsInOrder() const;

  // LetterCount::CombinationsOfSize()
  //
  // Returns a set of all `k`-letter combinations of the letters in this
  // `LetterCount`. Combinations are returned as alphabetized strings.
  absl::flat_hash_set<std::string> CombinationsOfSize(int k) const;

  // LetterCount::RegexMatchingContents()
  //
  // Returns regex that matches a letter only if this `LetterCount` contains
  // that letter.
  std::string RegexMatchingContents() const;

  // LetterCount::UniqueLetters()
  //
  // Returns an alphabetized string containing each letter that this
  // `LetterCount` contains one or more copies of.
  std::string UniqueLetters() const;

  //----------
  // Mutators

  // LetterCount::AddLetter()
  //
  // Increments the count of a given letter, doing so `i` times if `i` is
  // provided. Returns an error if `c` is not a letter or if `i` is negative. It
  // can safely be assumed that `counts_` remains unaltered by a call that
  // returns an error.
  absl::Status AddLetter(char c, int i);
  absl::Status AddLetter(char c) { return AddLetter(c, 1); }

  // LetterCount::AddLetters()
  //
  // Adds the letters in `s` to the count, ignoring non-letters.
  void AddLetters(absl::string_view s);

  // LetterCount::operator+=
  //
  // Addition/assignment funtions identically to `AddLetters()`, but also works
  // if `rhs` is a `LetterCount`.
  LetterCount &operator+=(const LetterCount &rhs);
  LetterCount &operator+=(const absl::string_view rhs) {
    return operator+=(LetterCount(rhs));
  }

  // LetterCount::RemoveLetter()
  //
  // Decrements the count of a given letter, doing so `i` times if `i` is
  // provided. Returns an error if `c` is not a letter, if `i` is negative, or
  // if there are fewer than `i` copies of `c` stored in this `LetterCount`. It
  // can safely be assumed that `counts_` remains unaltered by a call that
  // returns an error.
  absl::Status RemoveLetter(char c, int i);
  absl::Status RemoveLetter(char c) { return RemoveLetter(c, 1); }

  // LetterCount::RemoveLetters()
  //
  // Removes the letters in `s` from the count, ignoring all non-letters.
  // Returns an error if there are more copies of a letter in `s` than in the
  // `LetterCount`. It can safely be assumed that `counts_` remains unaltered by
  // a call that returns an error.
  absl::Status RemoveLetters(absl::string_view s);

  // LetterCount::operator-=
  //
  // Unlike `RemoveLetters()`, this does not return an error if the operation
  // leads to a negative count of a given letter. As a result, this should only
  // be called in situations in which the validity of the operation has already
  // been verified by a method such as `contains()`.
  LetterCount &operator-=(const LetterCount &rhs);
  LetterCount &operator-=(const absl::string_view rhs) {
    return operator-=(LetterCount(rhs));
  }

 private:
  // LetterCount::set_count()
  //
  // Sets the count of a given letter.
  void set_count(char c, int i);

  // LetterCount::FirstLetterNotContained()
  //
  // Returns `kBadC` if this `LetterCount` contains `other`. If it doesn't,
  // returns the first char in alphabetical order for which it fails.
  char FirstLetterNotContained(const LetterCount &other) const;

  //---------
  // Members

  std::vector<int> counts_;

  //------------------
  // Abseil functions

  template <typename H>
  friend H AbslHashValue(H h, const LetterCount &lc) {
    return H::combine(std::move(h), lc.counts_);
  }

  template <typename Sink>
  friend void AbslStringify(Sink &sink, const LetterCount &lc) {
    if (lc.empty()) return;
    std::vector<std::string> v;
    for (char c = 'a'; c <= 'z'; ++c) {
      if (int n = lc.count(c); n > 0)
        v.push_back(absl::StrCat(std::string(1, c), ":", n));
    }
    sink.Append(absl::StrCat("[", absl::StrJoin(v, ", "), "]"));
  }
};

//----------------------
// Non-member operators

bool operator==(const LetterCount &lhs, const LetterCount &rhs);
bool operator!=(const LetterCount &lhs, const LetterCount &rhs);
LetterCount operator+(const LetterCount &lhs, const LetterCount &rhs);
LetterCount operator-(const LetterCount &lhs, const LetterCount &rhs);

}  // namespace puzzmo

#endif