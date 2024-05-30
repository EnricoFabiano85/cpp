#pragma once

#include <iostream>

template<typename D>
class Expression
{
public:
  auto apply() const {return derived()->apply_impl();}

private:
  D const* derived() const {return static_cast<D const*>(this);}
  
  auto &apply_impl() const {std::cout << "APPLY NOT IMPLEMENETED" << std::endl;}
};