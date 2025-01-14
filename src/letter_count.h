#ifndef letter_count_h
#define letter_count_h

#include <cctype>
#include <string>
#include <vector>

struct LetterCount {
  std::vector<int> count;
  LetterCount() : count(26) {}
  explicit LetterCount(const std::string str);

  bool operator==(const LetterCount &other) const;

  LetterCount operator+(const LetterCount &other) const;

  template <typename H> friend H AbslHashValue(H h, const LetterCount &lc) {
    return H::combine(std::move(h), lc.count);
  }
};

#endif