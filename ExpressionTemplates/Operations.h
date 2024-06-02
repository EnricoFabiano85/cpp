#pragma once

#include "DiffMode.h"
#include "Expression.h"
#include <cmath>

struct Add
{                                     
  template<typename T1, typename T2> 
  static auto primal(T1 const &t1, T2 const &t2)
  { return t1.primal() + t2.primal(); }

  template<typename T1, typename T2> 
  static auto sensitivity(T1 const &t1, T2 const &t2)
  { return t1.sensitivity() + t2.sensitivity(); }
};

struct Subtract
{                                     
  template<typename T1, typename T2> 
  static auto primal(T1 const &t1, T2 const &t2)
  { return t1.primal() - t2.primal(); }

  template<typename T1, typename T2> 
  static auto sensitivity(T1 const &t1, T2 const &t2)
  { return t1.sensitivity() - t2.sensitivity(); } 
};

struct Multiply
{                                     
  template<typename T1, typename T2> 
  static auto primal(T1 const &t1, T2 const &t2)
  { return t1.primal() * t2.primal(); }

  template<typename T1, typename T2> 
  static auto sensitivity(T1 const &t1, T2 const &t2)
  { return t1.sensitivity()*t2.primal() + t1.primal()*t2.sensitivity(); }
};

struct Divide
{
  template<typename T1, typename T2> 
  static auto primal(T1 const &t1, T2 const &t2)
  { return t1.primal() / t2.primal(); }

  template<typename T1, typename T2> 
  static auto sensitivity(T1 const &t1, T2 const &t2)
  { return t1.sensitivity()/t2.primal() - t2.sensitivity()*t1.primal()/t2.primal()/t2.primal(); }
};

template<typename T1, typename Op, typename T2>
struct BinaryOp : public Expression<BinaryOp<T1, Op, T2>>
{
  T1 const &_t1;
  T2 const &_t2;
  static constexpr auto DMode = T1::DMode;

  BinaryOp(T1 const &t1, T2 const &t2) : _t1(t1)
                                       , _t2(t2)
  {}

  auto primal_impl() const
  { return Op::primal(_t1, _t2); }

  auto sensitivity_impl() const
  { return Op::sensitivity(_t1, _t2); }
};


struct Sin
{
  template<typename T> 
  static auto primal(T const &t)
  { return std::sin(t.primal()); }

  template<typename T> 
  static auto sensitivity(T const &t)
  { return std::cos(t.primal())*t.sensitivity(); }
};


template<typename Op, typename T>
struct UnaryOp : Expression<UnaryOp<Op, T>>
{
  T const &_t;
  static constexpr auto DMode = T::DMode;

  UnaryOp(T const &t) : _t(t)
  {}

  auto primal_impl() const
  { return Op::primal(_t); }

  auto sensitivity_impl() const
  { return Op::sensitivity(_t); }
};