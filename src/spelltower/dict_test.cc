#include "dict.h"

#include "absl/status/status_matchers.h"
#include "gtest/gtest.h"

namespace puzzmo::spelltower {
namespace {

TEST(DictTest, Init) {
  Dict dict;
  EXPECT_THAT(dict.Init(), absl_testing::IsOk());
}

}  // namespace
}  // namespace puzzmo::spelltower