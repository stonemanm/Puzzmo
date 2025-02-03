#ifndef letter_count_h
#define letter_count_h

#include <cctype>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"

namespace puzzmo {

class LetterCount {
public:
  LetterCount() : count(26) {}
  explicit LetterCount(absl::string_view s);

  absl::StatusOr<int> AddLetter(char c);
  absl::StatusOr<int> AddLetter(char c, int i);

  std::string CharsInOrder() const;

  int NumLetters(char c) const;

  absl::StatusOr<int> RemoveLetter(char c);
  absl::StatusOr<int> RemoveLetter(char c, int i);

  int Size() const;

  bool Valid() const;

  bool operator==(const LetterCount &other) const;
  bool operator<(const LetterCount &other) const;

  LetterCount &operator+=(const LetterCount &other);
  LetterCount operator+(const LetterCount &rhs) const;

  LetterCount &operator-=(const LetterCount &other);
  LetterCount operator-(const LetterCount &rhs) const;

  template <typename H> friend H AbslHashValue(H h, const LetterCount &lc) {
    return H::combine(std::move(h), lc.count);
  }

  template <typename Sink>
  friend void AbslStringify(Sink &sink, const LetterCount &lc) {
    absl::Format(&sink,
                 "[a:%d, b:%d, c:%d, d:%d, e:%d, f:%d, g:%d, h:%d, i:%d, j:"
                 "%d, k:%d, l:%d, m:%d, n:%d, o:%d, p:%d, q:%d, r:%d, s:%d, t:"
                 "%d, u:%d, v:%d, w:%d, x:%d, y:%d, z:%d]",
                 lc.count[0], lc.count[1], lc.count[2], lc.count[3],
                 lc.count[4], lc.count[5], lc.count[6], lc.count[7],
                 lc.count[8], lc.count[9], lc.count[10], lc.count[11],
                 lc.count[12], lc.count[13], lc.count[14], lc.count[15],
                 lc.count[16], lc.count[17], lc.count[18], lc.count[19],
                 lc.count[20], lc.count[21], lc.count[22], lc.count[23],
                 lc.count[24], lc.count[25]);
  }

private:
  std::vector<int> count;
};

} // namespace puzzmo

#endif