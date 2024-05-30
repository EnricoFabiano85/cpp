#pragma once

#include "Expression.h"
#include <cmath>

//Macro from https://github.com/DominicJones/snippets/blob/master/Cxx/expression_templates.cpp
#define GenerateBinaryOperator(OperatorType, Operator)      \
  struct OperatorType                                       \
  {                                                         \
    static auto apply(double const &t1, double const &t2)   \
    { return t1 Operator t2; };                             \
  };                                                        \

GenerateBinaryOperator(Add, +);
GenerateBinaryOperator(Subtract, -);
GenerateBinaryOperator(Multiply, *);
GenerateBinaryOperator(Divide, /);

#undef GenerateBinaryOperator

template<typename T1, typename Op, typename T2>
struct BinaryOp : public Expression<BinaryOp<T1, Op, T2>>
{
  T1 const &_t1;
  T2 const &_t2;

  BinaryOp(T1 const &t1, T2 const &t2) : _t1(t1)
                                       , _t2(t2)
  {}

  auto apply_impl() const
  { return Op::apply(_t1.apply(), _t2.apply()); }
};


struct Sin
{
  static auto apply(double const &t)
  { return std::sin(t); }
};


template<typename Op, typename T>
struct UnaryOp : Expression<UnaryOp<Op, T>>
{
  T const &_t1;

  UnaryOp(T const &t1) : _t1(t1)
  {}

  auto apply_impl() const
  { return Op::apply(_t1.apply()); }
};