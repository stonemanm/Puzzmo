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

  template <typename H> friend H AbslHashValue(H h, const LetterCount &lc) {
    return H::combine(std::move(h), lc.counts_);
  }

  template <typename Sink>
  friend void AbslStringify(Sink &sink, const LetterCount &lc) {
    absl::Format(&sink,
                 "[a:%d, b:%d, c:%d, d:%d, e:%d, f:%d, g:%d, h:%d, i:%d, j:"
                 "%d, k:%d, l:%d, m:%d, n:%d, o:%d, p:%d, q:%d, r:%d, s:%d, t:"
                 "%d, u:%d, v:%d, w:%d, x:%d, y:%d, z:%d]",
                 lc.counts_[0], lc.counts_[1], lc.counts_[2], lc.counts_[3],
                 lc.counts_[4], lc.counts_[5], lc.counts_[6], lc.counts_[7],
                 lc.counts_[8], lc.counts_[9], lc.counts_[10], lc.counts_[11],
                 lc.counts_[12], lc.counts_[13], lc.counts_[14], lc.counts_[15],
                 lc.counts_[16], lc.counts_[17], lc.counts_[18], lc.counts_[19],
                 lc.counts_[20], lc.counts_[21], lc.counts_[22], lc.counts_[23],
                 lc.counts_[24], lc.counts_[25]);
  }

private:
  std::vector<int> counts_;
};

} // namespace puzzmo

#endif