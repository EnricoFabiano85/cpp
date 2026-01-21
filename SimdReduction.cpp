#include <type_traits>
#include <vector>
#include <cassert>
#include <cstddef>
#include <experimental/simd>
#include <functional>
#include <iostream>
#include <numeric>
#include <span>
namespace stdx = std::experimental;

#include <algorithm>
#include <print>

auto linearSum(auto data)
{ 
  using T = decltype(data)::value_type;
  return std::reduce(data.begin(), data.end(), T{0.0}); 
}

auto simdSum(auto data)
{
  using T = decltype(data)::value_type;
  using V = stdx::native_simd<T>;
  std::size_t i = 0;
  typename V::value_type acc{};
  for ( ; i < data.size(); i += V::size())
      acc += stdx::reduce(V(&data[i], stdx::vector_aligned), std::plus{});

  for (; i < data.size(); ++i) acc += data[i];

  return acc;
}
 
int main()
{
    using V = stdx::native_simd<double>;

    // alignas(stdx::memory_alignment_v<V>) 
    auto data = std::vector<V::value_type>(1<<30);
    std::ranges::iota(data, 0);

    std::println("simd sum = {}", simdSum(std::span(data)));
    std::println("linear sum = {}", linearSum(std::span(data)));
}