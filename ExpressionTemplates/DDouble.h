#pragma once

#include "DiffConcepts.h"
#include "DiffMode.h"
#include "Expression.h"

template<DiffMode mode>
struct DDouble : public Expression
{
  static constexpr DiffMode DMode = mode;
  double _value{0};
  double _sensitivity{0};

  DDouble() = default;

  constexpr DDouble(double const value, double const sens) : _value(value)
                                                           , _sensitivity(sens)
  {}

  template<ForwardExpression T>
  DDouble(T const &rhs) : _value(rhs.primal())
                        , _sensitivity(rhs.sensitivity())
  {}

  auto &&primal_impl(this auto &&self)
  { return std::forward<decltype(self)>(self)._value; }

  auto &&sensitivity_impl(this auto &&self)
  { return std::forward<decltype(self)>(self)._sensitivity; };
};

template<>
struct DDouble<PRIMAL> : public Expression
{
  static constexpr DiffMode DMode = PRIMAL;
  double _value;

  constexpr DDouble(double const value) : _value(value)
  {}

  template<ForwardExpression T>
  DDouble(T const &rhs) : _value(rhs.primal())
  {}

  auto &&primal_impl(this auto &&self)
  { return std::forward<decltype(self)>(self)._value; }
};