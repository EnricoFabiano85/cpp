#pragma once

#include <iostream>

template<typename D>
class Expression
{
public:
  auto primal() const { return derived()->primal_impl(); }
  auto sensitivity() const { return derived()->sensitivity_impl(); }

private:
  D const* derived() const {return static_cast<D const*>(this);}
};