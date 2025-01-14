#include "letter_count.h"

#include <vector>

#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "gtest/gtest.h"

namespace puzzmo {
namespace {

TEST(LetterCountTest, EmptyConstructor) {
  LetterCount lc;
  EXPECT_EQ(lc.count.size(), 26);
  EXPECT_EQ(lc.count,
            std::vector<int>({0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                              0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}));
}

TEST(LetterCountTest, ConstructFromString) {
  std::string str = "Can you hear me?";
  LetterCount lc(str);
  EXPECT_EQ(lc.count, std::vector<int>({/* a = */ 2, /* b = */ 0,
                                        /* c = */ 1, /* d = */ 0,
                                        /* e = */ 2, /* f = */ 0,
                                        /* g = */ 0, /* h = */ 1,
                                        /* i = */ 0, /* j = */ 0,
                                        /* k = */ 0, /* l = */ 0,
                                        /* m = */ 1, /* n = */ 1,
                                        /* o = */ 1, /* p = */ 0,
                                        /* q = */ 0, /* r = */ 1,
                                        /* s = */ 0, /* t = */ 0,
                                        /* u = */ 1, /* v = */ 0,
                                        /* w = */ 0, /* x = */ 0,
                                        /* y = */ 1, /* z = */ 0}));
}

TEST(LetterCountTest, IsValid) {
  LetterCount lc;
  EXPECT_TRUE(lc.isValid());
  lc.count[0] = -3;
  EXPECT_FALSE(lc.isValid());
}

TEST(LetterCountTest, ToString) {
  LetterCount lc("Can you hear me?");
  EXPECT_EQ(lc.toString(), "aaceehmnoruy");
}

} // namespace
} // namespace puzzmo