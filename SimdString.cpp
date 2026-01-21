/*
Inspired from https://www.linkedin.com/posts/heriklima_cppperformance-avx2-simd-activity-7398873060159774720-FMAM?utm_source=share&utm_medium=member_desktop&rcm=ACoAABLqpd4BH4Ghaov3FjpvM5AutqBraf8mOEs
Compile: clang++ -std=c++23 -O3 -mavx2 (-DINTRINSIC) SimdString.cpp
*/

#include <chrono>
#include <cstddef>
#include <print>
#include <string>
#include <string_view>

#ifndef INTRINSIC
#include <experimental/simd>
namespace stdx = std::experimental;
#else
#include <immintrin.h>
#endif

int constexpr size = 1 << 20;
char constexpr target = 'z';

auto textA = std::string(size, 'a');

auto constexpr linearSearch(std::string_view string, char target) -> size_t
{
  auto const n = string.size();
  for(auto i = 0; i < n; ++i)
  {
    if (string[i] == target) return i;
  }

  return std::string_view::npos;
}

auto simdSearch(std::string_view string, char target) -> size_t
{
  auto const n = string.size();
  size_t i = 0;

#ifndef INTRINSIC

  using C = stdx::native_simd<char>;
  auto const simdTarget = C{'z'};
  for (; i+C::size() <= n; i += C::size())
  {
    auto const mySimdChunk = C{string.data()+i, stdx::element_aligned};
    auto const mask = (simdTarget == mySimdChunk);

    if (stdx::any_of(mask))
    {
      return stdx::find_first_set(mask)+i;
    }
  }

#else

  __m256i const vt = _mm256_set1_epi8(target);
  for (; i+32 <= n; i += 32)
  {
    __m256i vdata = _mm256_load_si256(reinterpret_cast<const __m256i*>(string.data()+i));
    __m256i cmp = _mm256_cmpeq_epi8(vdata, vt);
    auto mask = _mm256_movemask_epi8(cmp);

    if (mask != 0)
    {
      auto index = __builtin_ctz(mask);
      return i + static_cast<size_t>(index);
    }
  }

#endif

  for(; i < n; ++i)
  {
    if (string[i] == target) return i;
  }

  return std::string_view::npos;
}

auto measure(auto f)
{
  auto const start = std::chrono::high_resolution_clock::now();

  auto const result = f();

  auto const end = std::chrono::high_resolution_clock::now();
  auto const elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();

  return std::pair{result, elapsed};
}


int main()
{
  textA[size - 1] = 'z';

  for (auto i = 0; i <  5; ++i)
  {
    auto [linearResult, linearTime] = measure([] {return linearSearch(textA, target);});
    auto [simdResult, simdTime] = measure([] {return simdSearch(textA, target);});

    std::println("Normal search: index = {}, time = {} ms", linearResult, linearTime);
    std::println("Simd search: index = {}, time = {} ms", simdResult, simdTime);
  }

  return 0;
};