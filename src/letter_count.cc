#include "letter_count.h"

#include <numeric>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"

namespace puzzmo {
namespace {

void nCk(int start_at, int k, std::string &current, absl::string_view str,
         absl::flat_hash_set<std::string> &combinations) {
  if (k == 0) {
    combinations.insert(current);
    return;
  }
  for (int i = start_at; i < str.size() - (k - 1); ++i) {
    current.push_back(str[i]);
    nCk(i + 1, k - 1, current, str, combinations);
    current.pop_back();
  }
}

} // namespace

LetterCount::LetterCount(absl::string_view s) : counts_(26) {
  for (char c : s) {
    if (!std::isalpha(c)) {
      continue;
    }
    ++counts_[tolower(c) - 'a'];
  }
}

int LetterCount::Count(char c) const {
  return std::isalpha(c) ? counts_[std::tolower(c) - 'a'] : -1;
}

bool LetterCount::Contains(const LetterCount &other) const {
  for (int i = 0; i < 26; ++i) {
    char c = 'a' + i;
    if (Count(c) < other.Count(c)) {
      return false;
    }
  }
  return true;
}

bool LetterCount::Empty() const { return Size() == 0; }

bool LetterCount::Valid() const {
  for (int n : counts_) {
    if (n < 0) {
      return false;
    }
  }
  return true;
}

int LetterCount::Size() const {
  return std::accumulate(counts_.begin(), counts_.end(), 0);
}

std::string LetterCount::AnyCharRegex() const {
  std::string s = "[";
  for (int i = 0; i < 26; ++i) {
    if (counts_[i] > 0) {
      absl::StrAppend(&s, std::string(1, 'a' + i));
    }
  }
  return absl::StrCat(s, "]");
}

std::string LetterCount::CharsInOrder() const {
  std::string s = "";
  std::vector<int> temp(counts_);
  for (int i = 0; i < 26; ++i) {
    char c = 'a' + i;
    while (--temp[i] >= 0) {
      s += c;
    }
  }
  return s;
}

absl::flat_hash_set<std::string> LetterCount::CombinationsOfSize(int k) const {
  absl::flat_hash_set<std::string> combinations;
  std::string s = "";
  nCk(0, k, s, CharsInOrder(), combinations);
  return combinations;
}

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

  counts_[std::tolower(c) - 'a'] += i;
  return counts_[std::tolower(c) - 'a'];
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

  int n = counts_[std::tolower(c) - 'a'];
  if (n < i) {
    return absl::InvalidArgumentError(
        absl::StrCat("LetterCount contains only ", n, " '", std::string(1, c),
                     "'s, which is fewer than the ", i, " being removed."));
  }

  counts_[std::tolower(c) - 'a'] -= i;
  return counts_[std::tolower(c) - 'a'];
}

bool LetterCount::operator==(const LetterCount &other) const {
  for (int i = 0; i < 26; ++i) {
    if (counts_[i] != other.counts_[i])
      return false;
  }
  return true;
}

bool LetterCount::operator<(const LetterCount &other) const {
  for (int i = 0; i < 26; ++i) {
    if (counts_[i] != other.counts_[i]) {
      return counts_[i] > other.counts_[i];
    }
  }
  return false;
}

LetterCount &LetterCount::operator+=(const LetterCount &other) {
  for (int i = 0; i < 26; ++i) {
    counts_[i] += other.counts_[i];
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
    counts_[i] -= other.counts_[i];
  }
  return *this;
}

LetterCount LetterCount::operator-(const LetterCount &other) const {
  LetterCount ret(*this);
  ret -= other;
  return ret;
}

} // namespace puzzmo
