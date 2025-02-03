#include "letter_count.h"

#include <numeric>

#include "absl/strings/str_cat.h"

namespace puzzmo {

LetterCount::LetterCount(absl::string_view s) : count(26) {
  for (char c : s) {
    if (!std::isalpha(c)) {
      continue;
    }
    ++count[tolower(c) - 'a'];
  }
}

absl::StatusOr<int> LetterCount::AddLetter(char c) { return AddLetter(c, 1); }

absl::StatusOr<int> LetterCount::AddLetter(char c, int i) {
  if (!std::isalpha(c)) {
    return absl::InvalidArgumentError(
        absl::StrCat("Non-alphabetical character '", std::string(1, c),
                     "' cannot be passed to AddLetter."));
  }
  if (i <= 0) {
    return absl::InvalidArgumentError(
        absl::StrCat("AddLetter can only add a positive number of '",
                     std::string(1, c), "'s."));
  }

  count[std::tolower(c) - 'a'] += i;
  return count[std::tolower(c) - 'a'];
}

std::string LetterCount::CharsInOrder() const {
  std::string s = "";
  std::vector<int> temp(count);
  for (int i = 0; i < 26; ++i) {
    char c = 'a' + i;
    while (--temp[i] >= 0) {
      s += c;
    }
  }
  return s;
}

int LetterCount::NumLetters(char c) const {
  return std::isalpha(c) ? count[std::tolower(c) - 'a'] : -1;
}

absl::StatusOr<int> LetterCount::RemoveLetter(char c) {
  return RemoveLetter(c, 1);
}

absl::StatusOr<int> LetterCount::RemoveLetter(char c, int i) {
  if (!std::isalpha(c)) {
    return absl::InvalidArgumentError(
        absl::StrCat("Non-alphabetical character '", std::string(1, c),
                     "' cannot be passed to RemoveLetter."));
  }
  if (i <= 0) {
    return absl::InvalidArgumentError(
        absl::StrCat("RemoveLetter can only remove a positive number of '",
                     std::string(1, c), "'s."));
  }

  int n = count[std::tolower(c) - 'a'];
  if (n < i) {
    return absl::InvalidArgumentError(
        absl::StrCat("LetterCount contains only ", n, " '", std::string(1, c),
                     "'s, which is fewer than the ", i, " being removed."));
  }

  count[std::tolower(c) - 'a'] -= i;
  return count[std::tolower(c) - 'a'];
}

int LetterCount::Size() const {
  return std::accumulate(count.begin(), count.end(), 0);
}

bool LetterCount::Valid() const {
  for (int n : count) {
    if (n < 0) {
      return false;
    }
  }
  return true;
}

bool LetterCount::operator==(const LetterCount &other) const {
  for (int i = 0; i < 26; ++i) {
    if (count[i] != other.count[i])
      return false;
  }
  return true;
}

bool LetterCount::operator<(const LetterCount &other) const {
  for (int i = 0; i < 26; ++i) {
    if (count[i] != other.count[i]) {
      return count[i] > other.count[i];
    }
  }
  return false;
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

} // namespace puzzmo
