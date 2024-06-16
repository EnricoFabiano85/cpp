#pragma once

#include <iostream>

class Expression
{
public:
  auto primal(this auto &&self) { return self.primal_impl(); }
  auto sensitivity(this auto &&self) { return self.sensitivity_impl(); }
};