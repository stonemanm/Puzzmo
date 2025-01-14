#include "letter_count.h"

namespace puzzmo {

LetterCount::LetterCount(const std::string str) {
  for (const char c : str) {
    if (!isalpha(c))
      continue;
    ++count[tolower(c) - 'a'];
  }
}

bool LetterCount::operator==(const LetterCount &other) const {
  for (int i = 0; i < 26; ++i) {
    if (count[i] != other.count[i])
      return false;
  }
  return true;
}

LetterCount &LetterCount::operator+=(const LetterCount &other) {
  for (int i = 0; i < 26; ++i) {
    count[i] += other.count[i];
  }
  return *this;
}

LetterCount LetterCount::operator+(const LetterCount &other) const {
  LetterCount ret(*this);
  ret += other;
  return ret;
}

LetterCount operator+(const LetterCount &lhs, const LetterCount &rhs) {
  return lhs.operator+(rhs);
}

} // namespace puzzmo
