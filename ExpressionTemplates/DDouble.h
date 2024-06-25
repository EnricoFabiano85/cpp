#pragma once

#include "DiffConcepts.h"
#include "DiffMode.h"
#include "Expression.h"

template<DiffMode mode> struct DDouble;

template<>
struct DDouble<PRIMAL> : public Expression
{
  static constexpr DiffMode DMode = PRIMAL;
  double _value;

  constexpr DDouble(double const value) : _value(value)
  {}

  template<IsExpression T>
  DDouble(T const &rhs) : _value(rhs.primal())
  {}

  auto primal_impl() const
  { return _value; }
};

template<>
struct DDouble<TANGENT> : public Expression
{
  static constexpr DiffMode DMode = TANGENT;
  double _value;
  double _sensitivity;

  constexpr DDouble(double const value, double const sens) : _value(value)
                                                           , _sensitivity(sens)
  {}

  template<IsExpression T>
  DDouble(T const &rhs) : _value(rhs.primal())
                        , _sensitivity(rhs.sensitivity())
  {}

  auto primal_impl() const
  { return _value; }

  auto sensitivity_impl() const
  { return _sensitivity; }
};