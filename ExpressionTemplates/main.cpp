#include "DDouble.h"
#include "DiffMode.h"
#include "Operators.h"

#include <iostream>
#include <random>

int main()
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

  std::cout << "lTan " << lTan.sensitivity() << std::endl;

  // //-------------- ADJOINT
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

  std::cout << "-----> ADJ " << x.sensitivity() << " " << x1.sensitivity() << std::endl;

  std::cout << "----> DUALITY TEST:" << std::endl;
  std::cout << "----> " << seed*lTan.sensitivity() <<  std::endl;
  std::cout << "----> " << sens*x.sensitivity() + sens1*x1.sensitivity() << std::endl;
}
