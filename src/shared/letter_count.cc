#include "letter_count.h"

#include <algorithm>
#include <numeric>

#include "absl/strings/str_format.h"

namespace puzzmo {
namespace {

// Errors

constexpr absl::string_view kIsNotAlphaError =
    "Non-alphabetical character cannot be passed to %s.";
constexpr absl::string_view kNotEnoughError =
    "LetterCount contains %d '%c's, which is fewer than the %d to be removed.";
constexpr absl::string_view kNegativeQuantityError =
    "Quantity %d passed to %s cannot be negative.";

// When we sanitize a character, if it's not a letter, it's set to `kBadC`.
constexpr char kBadC = '\0';

// Helper functions

// Sanitizes the provided character.
char SanitizeChar(char c) { return std::isalpha(c) ? std::tolower(c) : kBadC; }

// Recursive function to help all combinations of size k
void nCk(int start_at, int k, std::string &current, absl::string_view str,
         absl::flat_hash_set<std::string> &combinations) {
  if (k <= 0) {
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

// Constructors

LetterCount::LetterCount(const absl::string_view s) : LetterCount() {
  for (char c : s)
    if (c = SanitizeChar(c); c != kBadC) ++counts_[c - 'a'];
}

// Accessors

int LetterCount::count(char c) const {
  return std::isalpha(c) ? counts_[std::tolower(c) - 'a'] : 0;
}

bool LetterCount::contains(const LetterCount &other) const {
  return FirstLetterNotContained(other) == kBadC;
}

bool LetterCount::empty() const { return size() == 0; }

int LetterCount::size() const {
  return std::accumulate(counts_.begin(), counts_.end(), 0);
}

std::string LetterCount::CharsInOrder() const {
  std::string s = "";
  for (char c = 'a'; c <= 'z'; ++c)
    for (int i = 0; i < count(c); ++i) s += c;
  return s;
}

absl::flat_hash_set<std::string> LetterCount::CombinationsOfSize(int k) const {
  absl::flat_hash_set<std::string> combinations;
  std::string s = "";
  nCk(0, k, s, CharsInOrder(), combinations);
  return combinations;
}

char LetterCount::FirstLetterNotContained(const LetterCount &other) const {
  for (char c = 'a'; c <= 'z'; ++c)
    if (count(c) < other.count(c)) return c;
  return kBadC;
}

std::string LetterCount::RegexMatchingContents() const {
  return absl::StrCat("[", UniqueLetters(), "]");
}

std::string LetterCount::UniqueLetters() const {
  std::string s = "";
  for (char c = 'a'; c <= 'z'; ++c)
    if (count(c) > 0) s.push_back(c);
  return s;
}

// Mutators

void LetterCount::set_count(char c, int i) {
  c = SanitizeChar(c);
  if (c != kBadC) counts_[c - 'a'] = i;
}

absl::Status LetterCount::AddLetter(char c, int i) {
  c = SanitizeChar(c);
  if (c == kBadC)
    return absl::InvalidArgumentError(
        absl::StrFormat(kIsNotAlphaError, "AddLetter()"));

  if (i < 0)
    return absl::InvalidArgumentError(
        absl::StrFormat(kNegativeQuantityError, i, "AddLetter()"));

  if (i > 0) set_count(c, count(c) + i);
  return absl::OkStatus();
}

void LetterCount::AddLetters(absl::string_view s) { *this += s; }

absl::Status LetterCount::RemoveLetter(char c, int i) {
  c = SanitizeChar(c);
  if (c == kBadC)
    return absl::InvalidArgumentError(
        absl::StrFormat(kIsNotAlphaError, "RemoveLetter()"));

  if (i < 0)
    return absl::InvalidArgumentError(
        absl::StrFormat(kNegativeQuantityError, i, "RemoveLetter()"));

  if (count(c) < i)
    return absl::InvalidArgumentError(
        absl::StrFormat(kNotEnoughError, count(c), c, i));

  if (i > 0) set_count(c, count(c) - i);
  return absl::OkStatus();
}

absl::Status LetterCount::RemoveLetters(absl::string_view s) {
  LetterCount other(s);
  if (char c = FirstLetterNotContained(other); c != kBadC)
    return absl::InvalidArgumentError(
        absl::StrFormat(kNotEnoughError, count(c), c, other.count(c)));

  *this -= s;
  return absl::OkStatus();
}

// Operator overloads

LetterCount &LetterCount::operator+=(const LetterCount &rhs) {
  for (char c = 'a'; c <= 'z'; ++c) set_count(c, count(c) + rhs.count(c));
  return *this;
}

LetterCount &LetterCount::operator-=(const LetterCount &rhs) {
  for (char c = 'a'; c <= 'z'; ++c)
    set_count(c, std::max(count(c) - rhs.count(c), 0));
  return *this;
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

bool operator==(const LetterCount &lhs, const LetterCount &rhs) {
  for (char c = 'a'; c <= 'z'; ++c)
    if (lhs.count(c) != rhs.count(c)) return false;
  return true;
}

bool operator!=(const LetterCount &lhs, const LetterCount &rhs) {
  return !(lhs == rhs);
}

}  // namespace puzzmo
