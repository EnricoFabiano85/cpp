#pragma once

struct Expression
{
  auto primal(this auto &&self) { return self.primal_impl(); }
  auto sensitivity(this auto &&self) { return self.sensitivity_impl(); }
  auto adjoint(this auto &&self) -> decltype(auto) { return self.adjoint_impl(); }
};