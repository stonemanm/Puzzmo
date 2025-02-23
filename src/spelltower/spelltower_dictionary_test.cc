#include "spelltower_dictionary.h"

#include "absl/status/status_matchers.h"
#include "gtest/gtest.h"

namespace puzzmo {
namespace {

TEST(SpelltowerDictionaryTest, Init) {
  SpelltowerDictionary dict;
  EXPECT_THAT(dict.Init(), absl_testing::IsOk());
}

}  // namespace
}  // namespace puzzmo