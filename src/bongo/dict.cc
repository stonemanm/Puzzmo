#include "dict.h"

#include <fstream>
#include <string>

#include "absl/flags/flag.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "re2/re2.h"

ABSL_FLAG(std::string, bongo_words_path, "data/words_bongo.txt",
          "Input file containing all legal words for Bongo.");
ABSL_FLAG(std::string, common_bongo_words_path, "data/words_bongo_common.txt",
          "Input file containing all \"common\" words in Bongo. (Common words "
          "are worth 1.3x when scored.)");

namespace puzzmo::bongo {

using SearchableWords = Dict::SearchableWords;

bool operator==(const SearchKey& lhs, const SearchKey& rhs) {
  return lhs.length == rhs.length && lhs.letters == rhs.letters;
}
bool operator!=(const SearchKey& lhs, const SearchKey& rhs) {
  return !(lhs == rhs);
}

absl::StatusOr<Dict> Dict::LoadFromFiles() {
  absl::StatusOr<absl::flat_hash_set<std::string>> valid_words =
      TryReadingInWords(false);
  if (!valid_words.ok()) return valid_words.status();

  absl::StatusOr<absl::flat_hash_set<std::string>> common_words =
      TryReadingInWords(true);
  if (!common_words.ok()) return common_words.status();

  absl::StatusOr<SearchableWords> searchable_words =
      TryReadingInAndSortingWords();
  if (!searchable_words.ok()) return searchable_words.status();

  return Dict(*std::move(valid_words), *std::move(common_words),
              *std::move(searchable_words));
}

bool Dict::IsCommonWord(absl::string_view word) const {
  return common_words_.contains(word);
}

bool Dict::IsValidWord(absl::string_view word) const {
  return valid_words_.contains(word);
}

absl::flat_hash_set<std::string> Dict::GetMatchingWords(
    const SearchOptions& options) const {
  absl::flat_hash_set<std::string> matches;

  for (const auto& [key, anagrams] : searchable_words_) {
    if (key.length < options.min_length || options.max_length < key.length ||
        !key.letters.contains(options.min_letters))
      continue;
    if (!options.max_letters.empty() &&
        !options.max_letters.contains(key.letters))
      continue;

    for (const std::string& word : anagrams) {
      if (!options.matching_regex.empty() &&
          !RE2::FullMatch(word, options.matching_regex))
        continue;

      matches.insert(word);
    }
  }

  return matches;
}

absl::StatusOr<absl::flat_hash_set<std::string>> Dict::TryReadingInWords(
    bool common) {
  std::string path = absl::GetFlag(common ? FLAGS_common_bongo_words_path
                                          : FLAGS_bongo_words_path);
  std::ifstream file(path);
  if (!file.is_open()) {
    return absl::InvalidArgumentError(
        absl::StrCat("Error: Could not open ", path));
  }
  absl::flat_hash_set<std::string> words;
  std::string line;
  while (std::getline(file, line)) {
    words.insert(line);
  }
  file.close();
  return words;
}

absl::StatusOr<SearchableWords> Dict::TryReadingInAndSortingWords() {
  std::string path = absl::GetFlag(FLAGS_bongo_words_path);
  std::ifstream file(path);
  if (!file.is_open()) {
    return absl::InvalidArgumentError(
        absl::StrCat("Error: Could not open ", path));
  }
  SearchableWords words;
  std::string line;
  while (std::getline(file, line)) {
    words[{.length = line.length(), .letters = LetterCount(line)}].insert(line);
  }
  file.close();

  return words;
}

}  // namespace puzzmo::bongo