#pragma once

#include "DiffMode.h"
#include "Expression.h"
#include <cmath>
#include <ctime>
#include <type_traits>

struct Add
{                                     
  template<typename T1, typename T2> 
  static auto primal(T1 const &t1, T2 const &t2)
  { return t1.primal() + t2.primal(); }

  template<typename T1, typename T2> 
  static auto sensitivity(T1 const &t1, T2 const &t2)
  { return t1.sensitivity() + t2.sensitivity(); }

  template<typename T1, typename T2>
  static auto adjoint(T1 &t1, T2 &t2, double const adjSens)
  { 
    t1.adjoint() += adjSens; 
    t2.adjoint() += adjSens;
  }
};

struct Subtract
{                                     
  template<typename T1, typename T2> 
  static auto primal(T1 const &t1, T2 const &t2)
  { return t1.primal() - t2.primal(); }

  template<typename T1, typename T2> 
  static auto sensitivity(T1 const &t1, T2 const &t2)
  { return t1.sensitivity() - t2.sensitivity(); } 

  template<typename T1, typename T2>
  static auto adjoint(T1 &t1, T2 &t2, double const adjSens)
  { 
    t1.adjoint() -= adjSens; 
    t2.adjoint() -= adjSens;
  }
};

struct Multiply
{                                     
  template<typename T1, typename T2> 
  static auto primal(T1 const &t1, T2 const &t2)
  { return t1.primal() * t2.primal(); }

  template<typename T1, typename T2> 
  static auto sensitivity(T1 const &t1, T2 const &t2)
  { return t1.sensitivity()*t2.primal() + t1.primal()*t2.sensitivity(); }

  template<typename T1, typename T2>
  static auto adjoint(T1 &t1, T2 &t2, double const adjSens)
  { 
    t1.adjoint() += t2.primal()*adjSens; 
    t2.adjoint() += t1.primal()*adjSens;
  }
};

struct Divide
{
  template<typename T1, typename T2> 
  static auto primal(T1 const &t1, T2 const &t2)
  { return t1.primal() / t2.primal(); }

  template<typename T1, typename T2> 
  static auto sensitivity(T1 const &t1, T2 const &t2)
  { return t1.sensitivity()/t2.primal() - t2.sensitivity()*t1.primal()/t2.primal()/t2.primal(); }

  template<typename T1, typename T2>
  static auto adjoint(T1 &t1, T2 &t2, double const adjSens)
  { 
    t1.adjoint() += adjSens/t2.primal(); 
    t2.adjoint() -= adjSens*t1.primal()/t2.primal()/t2.primal();
  }
};

template<typename T1, typename Op, typename T2>
struct BinaryOp : public Expression
{
  static constexpr auto DMode = T1::DMode;
  using Type1 = std::conditional_t<DMode != DiffMode::ADJOINT, T1 const, T1>;
  using Type2 = std::conditional_t<DMode != DiffMode::ADJOINT, T2 const, T2>;

  Type1 &_t1;
  Type2 &_t2;
  double _tmpSens{0};


  BinaryOp(Type1 &t1, Type2 &t2) : _t1(t1)
                                 , _t2(t2)
  {}

  ~BinaryOp()
  { if constexpr (DMode == ADJOINT) { Op::adjoint(_t1, _t2, _tmpSens); } }

  auto primal_impl() const
  { return Op::primal(_t1, _t2); }

  auto sensitivity_impl() const
  { return Op::sensitivity(_t1, _t2); }

  auto &adjoint_impl()
  { return _tmpSens; }
};


struct Sin
{
  template<typename T> 
  static auto primal(T const &t)
  { return std::sin(t.primal()); }

  template<typename T> 
  static auto sensitivity(T const &t)
  { return std::cos(t.primal())*t.sensitivity(); }

  template<typename T>
  static auto adjoint(T &t, double const adjSens)
  { t.adjoint() += std::cos(t.primal())*adjSens; }
};


template<typename Op, typename T>
struct UnaryOp : Expression
{
  static constexpr auto DMode = T::DMode;
  using Type = std::conditional_t<DMode != DiffMode::ADJOINT, T const, T>;

  Type &_t;
  double _tmpSens{0};

  UnaryOp(Type &t) : _t(t)
  {}

  ~UnaryOp()
  { if constexpr (DMode == ADJOINT) { Op::adjoint(_t, _tmpSens); } }

  auto primal_impl() const
  { return Op::primal(_t); }

  auto sensitivity_impl() const
  { return Op::sensitivity(_t); }

  auto &adjoint_impl()
  { return _tmpSens; }
};