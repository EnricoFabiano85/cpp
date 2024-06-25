#pragma once

#include <utility>

struct Expression
{
  decltype(auto) primal(this auto &&self) 
  { return self.primal_impl(); }

  decltype(auto) sensitivity(this auto &&self) 
  { return std::forward<decltype(self)>(self).sensitivity_impl();  }
};