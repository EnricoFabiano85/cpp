#pragma once

#include "Operations.h"

template<typename T1, typename T2>
auto operator+(T1 const &t1, T2 const &t2)
{
  return BinaryOp<T1, Add, T2>(t1, t2);
}

template<typename T1, typename T2>
auto operator/(T1 const &t1, T2 const &t2)
{
  return BinaryOp<T1, Divide, T2>(t1, t2);
}

template<typename T>
auto sin(T const &t)
{ 
  return UnaryOp<Sin, T>(t);
}