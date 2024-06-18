#include "DDouble.h"
#include "Operators.h"

#include "/home/enrico/Documents/coding/googletest/googletest/include/gtest/gtest.h"
#include <random>

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

TEST(AdTest, Adjoint)
{
  std::random_device rd{};
  std::mt19937 gen{rd()};
  std::uniform_real_distribution<> distribution(1.0, 10.0);
  auto const sens = distribution(gen);
  auto const sens1 = distribution(gen);

  //-------------- TANGENT
  DDouble<TANGENT> lTan;
  {
    DDouble<TANGENT> x{2, sens};
    DDouble<TANGENT> x1{1, sens1};
    DDouble<TANGENT> y{5, 0};
    DDouble<TANGENT> z{10, 0};
    DDouble<TANGENT> k{42, 0};
    auto const expression = x+x1;
    auto const expression1 = z+y;
    auto const expression2 = expression1 / expression;
    auto const expression3 = sin(expression2);
    lTan = k+expression3;
  }

  //-------------- ADJOINT
  double constexpr seed = 1;


  DDouble<ADJOINT> x{2, 0};
  DDouble<ADJOINT> x1{1, 0};
  DDouble<ADJOINT> y{5, 0};
  DDouble<ADJOINT> z{10, 0};
  DDouble<ADJOINT> k{42, 0};

  {
    auto const sum1 = x+x1;
    auto const sum2 = z+y;
    auto const ratio = sum2/sum1;
    auto const trig = sin(ratio);
    auto prod2 = k + trig;
    prod2._tmpSens = seed;
  }

  auto const dual1 = seed*lTan.sensitivity();
  auto const dual2 = sens*x.sensitivity() + sens1*x1.sensitivity();
  auto constexpr tol = 1e-13;

  EXPECT_NEAR(dual1, dual2, tol);
}