#include "dict.h"

#include <fstream>
#include <string>

#include "absl/flags/flag.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_format.h"
#include "re2/re2.h"

ABSL_FLAG(std::string, valid_file_path, "data/words_bongo.txt",
          "Input file containing all legal words for Bongo.");
ABSL_FLAG(std::string, common_file_path, "data/words_bongo_common.txt",
          "Input file containing all \"common\" words in Bongo. (Common words "
          "are worth 1.3x when scored.)");

namespace puzzmo::bongo {

constexpr absl::string_view kFileError = "Error: Could not open %s.";

// Constructors

absl::StatusOr<Dict> Dict::LoadFromFiles() {
  std::string line;

  // Load the valid words from file.
  std::ifstream valid_file(absl::GetFlag(FLAGS_valid_file_path));
  if (!valid_file.is_open())
    return absl::NotFoundError(
        absl::StrFormat(kFileError, absl::GetFlag(FLAGS_valid_file_path)));
  absl::flat_hash_map<LetterCount, absl::flat_hash_set<std::string>> words;
  while (std::getline(valid_file, line)) words[LetterCount(line)].insert(line);
  valid_file.close();

  // Load the common words from file.
  std::ifstream common_file(absl::GetFlag(FLAGS_common_file_path));
  if (!common_file.is_open())
    return absl::NotFoundError(
        absl::StrFormat(kFileError, absl::GetFlag(FLAGS_common_file_path)));
  absl::flat_hash_map<LetterCount, absl::flat_hash_set<std::string>>
      common_words;
  while (std::getline(common_file, line))
    common_words[LetterCount(line)].insert(line);
  common_file.close();

  return Dict(std::move(words), std::move(common_words));
}

Dict::Dict(const absl::flat_hash_set<std::string>& words,
           const absl::flat_hash_set<std::string>& common_words) {
  for (const std::string& word : words) words_[LetterCount(word)].insert(word);
  for (const std::string& word : common_words)
    common_words_[LetterCount(word)].insert(word);
}

// Accessors

bool Dict::contains(absl::string_view word) const {
  LetterCount key(word);
  return words_.contains(key) && words_.at(key).contains(word);
}

bool Dict::IsCommonWord(absl::string_view word) const {
  LetterCount key(word);
  return common_words_.contains(key) && common_words_.at(key).contains(word);
}

absl::flat_hash_set<std::string> Dict::WordsMatchingParameters(
    const SearchParameters& params) const {
  absl::flat_hash_set<std::string> matches;

  for (const auto& [letter_count, anagrams] : words_) {
    const int len = letter_count.size();
    if (len < params.min_length) continue;
    if (len > params.max_length) continue;
    if (!letter_count.contains(params.min_letters)) continue;
    if (!params.max_letters.empty() &&
        !params.max_letters.contains(letter_count))
      continue;

    for (const std::string& word : anagrams) {
      if (!params.matching_regex.empty() &&
          !RE2::FullMatch(word, params.matching_regex))
        continue;
      matches.insert(word);
    }
  }
  return matches;
}

}  // namespace puzzmo::bongo