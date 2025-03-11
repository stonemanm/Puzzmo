#include "letter_count.h"

#include <algorithm>
#include <numeric>

namespace puzzmo {
namespace {

// Recursive function to help all combinations of size k
void nCk(int start_at, int k, std::string &current, absl::string_view str,
         absl::flat_hash_set<std::string> &combinations) {
  if (k < 0) {
    combinations.insert(current);
    return;
  }
  for (int i = start_at; i < str.size() - (k - 1); ++i) {
    current.push_back(str[i]);
    nCk(i + 1, k - 1, current, str, combinations);
    current.pop_back();
  }
}

}  // namespace

LetterCount::LetterCount(absl::string_view s) : counts_(26) {
  for (char c : s) {
    if (!std::isalpha(c)) continue;
    ++counts_[tolower(c) - 'a'];
  }
}

absl::StatusOr<int> LetterCount::AddLetter(char c, int i) {
  if (!std::isalpha(c)) {
    return absl::InvalidArgumentError(
        absl::StrCat("Non-alphabetical character '", std::string(1, c),
                     "' cannot be passed to AddLetter."));
  }
  if (i < 0)
    return absl::InvalidArgumentError(
        "Quantity passed to AddLetter must be nonnegative.");

  c = std::tolower(c);
  if (i > 0) set_count(c, count(c) + i);
  return count(c);
}

void LetterCount::AddLetters(absl::string_view s) { *this += s; }

absl::StatusOr<int> LetterCount::RemoveLetter(char c, int i) {
  if (!std::isalpha(c)) {
    return absl::InvalidArgumentError(
        absl::StrCat("Non-alphabetical character '", std::string(1, c),
                     "' cannot be passed to RemoveLetter."));
  }
  if (i < 0) {
    return absl::InvalidArgumentError(
        absl::StrCat("RemoveLetter can only remove a positive number of '",
                     std::string(1, c), "'s."));
  }
  c = std::tolower(c);
  if (count(c) < i) {
    return absl::InvalidArgumentError(absl::StrCat(
        "LetterCount contains only ", count(c), " '", std::string(1, c),
        "'s, which is fewer than the ", i, " being removed."));
  }
  if (i > 0) set_count(c, count(c) - i);
  return count(c);
}

absl::Status LetterCount::RemoveLetters(absl::string_view s) {
  if (!this->contains(s))
    return absl::InvalidArgumentError(
        absl::StrCat("LetterCount does not contain all of the letters in ", s));
  *this -= s;
  return absl::OkStatus();
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

std::string LetterCount::UniqueLetters() const {
  std::string s = "";
  for (int i = 0; i < 26; ++i)
    if (counts_[i] > 0) s.push_back('a' + i);
  return s;
}

absl::flat_hash_set<std::string> LetterCount::CombinationsOfSize(int k) const {
  absl::flat_hash_set<std::string> combinations;
  std::string s = "";
  nCk(0, k, s, CharsInOrder(), combinations);
  return combinations;
}

std::string LetterCount::RegexMatchingContents() const {
  return absl::StrCat("[", UniqueLetters(), "]");
}

/** * * * * * * * * *
 * Container methods *
 * * * * * * * * * **/

bool LetterCount::contains(const LetterCount &other) const {
  if (other.empty()) return true;
  for (int i = 0; i < 26; ++i) {
    char c = 'a' + i;
    if (count(c) < other.count(c)) return false;
  }
  return true;
}

bool LetterCount::empty() const { return size() == 0; }

int LetterCount::size() const {
  return std::accumulate(counts_.begin(), counts_.end(), 0);
}

/** * * * * * * * * * * *
 * Accessors & mutators *
 * * * * * * * * * * * **/

void LetterCount::set_count(char c, int i) {
  if (!std::isalpha(c)) return;
  counts_[std::tolower(c) - 'a'] = i;
}

int LetterCount::count(char c) const {
  return std::isalpha(c) ? counts_[std::tolower(c) - 'a'] : 0;
}

/** * * * * * * * * * * *
 * Overloaded operators *
 * * * * * * * * * * * **/

LetterCount &LetterCount::operator+=(const LetterCount &rhs) {
  for (int i = 0; i < 26; ++i) {
    set_count('a' + i, count('a' + i) + rhs.count('a' + i));
  }
  return *this;
}

LetterCount &LetterCount::operator-=(const LetterCount &rhs) {
  for (int i = 0; i < 26; ++i) {
    set_count('a' + i, std::max(count('a' + i) - rhs.count('a' + i), 0));
  }
  return *this;
}

bool operator==(const LetterCount &lhs, const LetterCount &rhs) {
  for (int i = 0; i < 26; ++i) {
    if (lhs.count('a' + i) != rhs.count('a' + i)) return false;
  }
  return true;
}

bool operator!=(const LetterCount &lhs, const LetterCount &rhs) {
  return !(lhs == rhs);
}

LetterCount operator+(const LetterCount &lhs, const LetterCount &rhs) {
  LetterCount result = lhs;
  result += rhs;
  return result;
}

LetterCount operator-(const LetterCount &lhs, const LetterCount &rhs) {
  LetterCount result = lhs;
  result -= rhs;
  return result;
}

}  // namespace puzzmo
