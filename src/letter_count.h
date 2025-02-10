#ifndef letter_count_h
#define letter_count_h

#include <string>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"

namespace puzzmo {

class LetterCount {
 public:
  LetterCount() : counts_(26) {};
  explicit LetterCount(absl::string_view s);

  int Count(char c) const;

  bool Contains(const LetterCount &other) const;
  bool Contains(absl::string_view other) const {
    return Contains(LetterCount(other));
  }

  bool Empty() const;
  bool Valid() const;

  int Size() const;

  std::string AnyCharRegex() const;
  std::string CharsInOrder() const;
  absl::flat_hash_set<std::string> CombinationsOfSize(int k) const;

  absl::StatusOr<int> AddLetter(char c) { return AddLetter(c, 1); }
  absl::StatusOr<int> AddLetter(char c, int i);
  absl::StatusOr<int> RemoveLetter(char c) { return RemoveLetter(c, 1); }
  absl::StatusOr<int> RemoveLetter(char c, int i);

  bool operator==(const LetterCount &other) const;
  bool operator<(const LetterCount &other) const;
  LetterCount &operator+=(const LetterCount &other);
  LetterCount operator+(const LetterCount &rhs) const;
  LetterCount &operator-=(const LetterCount &other);
  LetterCount operator-(const LetterCount &rhs) const;

  template <typename H>
  friend H AbslHashValue(H h, const LetterCount &lc) {
    return H::combine(std::move(h), lc.counts_);
  }

  template <typename Sink>
  friend void AbslStringify(Sink &sink, const LetterCount &lc) {
    absl::Format(&sink, "%w", lc.StringifyHelper());
  }

 private:
  std::vector<int> counts_;

  std::string StringifyHelper() const;
};

}  // namespace puzzmo

#endif