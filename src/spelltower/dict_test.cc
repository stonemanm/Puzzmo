#include "dict.h"

#include "absl/status/status_matchers.h"
#include "gtest/gtest.h"

namespace puzzmo::spelltower {
namespace {

TEST(DictTest, LoadDictFromSerializedTrie) {
  auto dict = Dict::LoadDictFromSerializedTrie();
  EXPECT_THAT(dict.status(), absl_testing::IsOk());
}

}  // namespace
}  // namespace puzzmo::spelltower