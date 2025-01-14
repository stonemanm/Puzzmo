#include "letter_count.h"

namespace puzzmo {

bool LetterCount::operator==(const LetterCount &other) const {
  for (int i = 0; i < 26; ++i) {
    if (count[i] != other.count[i])
      return false;
  }
  return true;
}

LetterCount LetterCount::operator+(const LetterCount &other) const {
  LetterCount ans;
  for (int i = 0; i < 26; ++i) {
    ans.count[i] = count[i] + other.count[i];
  }
  return ans;
}

} // namespace puzzmo