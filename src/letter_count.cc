#include "letter_count.h"

#include "absl/log/log.h"
#include "absl/strings/str_cat.h"

namespace puzzmo {

LetterCount::LetterCount(const std::string str) : count(26) {
  for (char c : str) {
    if (!isalpha(c)) {
      continue;
    }
    ++count[tolower(c) - 'a'];
  }
}

bool LetterCount::isValid() const {
  for (int i : count) {
    if (i < 0)
      return false;
  }
  return true;
}

std::string LetterCount::toString() const {
  std::string str = "";
  std::vector<int> temp(count);
  for (int i = 0; i < 26; ++i) {
    char c = 'a' + i;
    while (--temp[i] >= 0) {
      str += c;
    }
  }
  return str;
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

// LetterCount operator+(const LetterCount &lhs, const LetterCount &rhs) {
//   return lhs.operator+(rhs);
// }

LetterCount &LetterCount::operator-=(const LetterCount &other) {
  for (int i = 0; i < 26; ++i) {
    count[i] -= other.count[i];
  }
  return *this;
}

LetterCount LetterCount::operator-(const LetterCount &other) const {
  LetterCount ret(*this);
  ret -= other;
  return ret;
}

// LetterCount operator-(const LetterCount &lhs, const LetterCount &rhs) {
//   return lhs.operator-(rhs);
// }

} // namespace puzzmo
