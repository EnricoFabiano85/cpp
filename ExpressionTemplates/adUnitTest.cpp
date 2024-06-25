#include "DDouble.h"
#include "Operators.h"

#include "/home/enrico/Documents/coding/googletest/googletest/include/gtest/gtest.h"

/*
g++ -std=c++23 -O3 adUnitTest.cpp -I"../../googletest/googletest/include/gtest" /usr/local/lib/libgtest.a /usr/local/lib/libgtest_main.a
*/

TEST(AdTest, Tangent)
{
  DDouble<TANGENT> x{2, .322};
  DDouble<TANGENT> x1{1, .14};
  DDouble<TANGENT> y{5, 0};
  DDouble<TANGENT> z{10, 0};
  DDouble<TANGENT> k{42, 0};

  DDouble<TANGENT> t = k+sin((z+y)/(x+x1));

  EXPECT_EQ(41.04107572533686, t.primal());
  EXPECT_EQ(-0.2184198828066842, t.sensitivity());
}