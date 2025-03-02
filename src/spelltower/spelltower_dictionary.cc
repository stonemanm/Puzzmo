#include "spelltower_dictionary.h"

#include <fstream>
#include <string>

#include "absl/flags/flag.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "re2/re2.h"

ABSL_FLAG(std::string, spelltower_words_path, "data/words_puzzmo.txt",
          "Input file containing all legal words for Spellotwer.");

namespace puzzmo::spelltower {

using SearchableWords = SpelltowerDictionary::SearchableWords;

absl::Status SpelltowerDictionary::Init() {
  // Initialize common_words_
  if (auto common_words = TryReadingInWords(); !common_words.ok()) {
    return common_words.status();
  } else {
    common_words_ = *std::move(common_words);
  }

  // Initialize valid_words_
  if (auto valid_words = TryReadingInWords(); !valid_words.ok()) {
    return valid_words.status();
  } else {
    valid_words_ = *std::move(valid_words);
  }

  // Initialize searchable_words_
  if (absl::StatusOr<SearchableWords> searchable_words =
          TryReadingInAndSortingWords();
      !searchable_words.ok()) {
    return searchable_words.status();
  } else {
    searchable_words_ = *std::move(searchable_words);
  }

  return absl::OkStatus();
}

bool SpelltowerDictionary::contains(absl::string_view word) const {
  return common_words_.contains(word);
}

absl::flat_hash_set<std::string> SpelltowerDictionary::GetMatchingWords(
    const SearchOptions& options) const {
  absl::flat_hash_set<std::string> matches;

  for (const auto& [len, submap] : searchable_words_) {
    if (len < options.min_length || options.max_length < len) continue;

    for (const auto& [letter_count, anagrams] : submap) {
      if (!letter_count.contains(options.min_letters)) continue;
      if (!options.max_letters.empty() &&
          !options.max_letters.contains(letter_count))
        continue;

      for (const std::string& word : anagrams) {
        if (!options.matching_regex.empty() &&
            !RE2::FullMatch(word, options.matching_regex))
          continue;

        matches.insert(word);
      }
    }
  }
  return matches;
}

absl::StatusOr<absl::flat_hash_set<std::string>>
SpelltowerDictionary::TryReadingInWords() const {
  std::string path = absl::GetFlag(FLAGS_spelltower_words_path);
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

absl::StatusOr<SearchableWords>
SpelltowerDictionary::TryReadingInAndSortingWords() const {
  std::string path = absl::GetFlag(FLAGS_spelltower_words_path);
  std::ifstream file(path);
  if (!file.is_open()) {
    return absl::InvalidArgumentError(
        absl::StrCat("Error: Could not open ", path));
  }
  SearchableWords words;
  std::string line;
  while (std::getline(file, line)) {
    words[line.length()][LetterCount(line)].insert(line);
  }
  file.close();

  return words;
}

}  // namespace puzzmo::spelltower