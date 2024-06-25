#pragma once

#include "Operations.h"

template<typename T1, typename T2>
auto operator+(T1 const &t1, T2 const &t2)
{ 
  if constexpr (T1::DMode != ADJOINT)
  { return BinaryOp<T1, Add, T2>(t1, t2); }
  else 
  { return BinaryOp<T1, Add, T2>(const_cast<T1&>(t1), const_cast<T2&>(t2)); }
}

template<typename T1, typename T2>
auto operator*(T1 const &t1, T2 const &t2)
{ 
  if constexpr (T1::DMode != ADJOINT)
  { return BinaryOp<T1, Multiply, T2>(t1, t2); }
  else 
  { return BinaryOp<T1, Multiply, T2>(const_cast<T1&>(t1), const_cast<T2&>(t2)); }
}
  

template<typename T1, typename T2>
auto operator/(T1 const &t1, T2 const &t2)
{ 
  if constexpr (T1::DMode != ADJOINT)
  { return BinaryOp<T1, Divide, T2>(t1, t2); }
  else 
  { return BinaryOp<T1, Divide, T2>(const_cast<T1&>(t1), const_cast<T2&>(t2)); }
}


template<typename T>
auto sin(T const &t)
{ 
  if constexpr (T::DMode != ADJOINT)
  { return UnaryOp<Sin, T>(t); } 
  else
  { return UnaryOp<Sin, T>(const_cast<T&>(t)); }
}  
