/*
Conclusions: 
1) contiguous access + hardware prefetching is unbeatable
2) Unstructured-like access pattern is SIGNIFICANTLY slower (as expected)
3) SW prefetching helps with unstructured access, but distance needs to be tuned
4) AVX512 gather is the second fastest approach
5) AVX512 + SW prefetching as slow as 3) (--> no SIMD advantage)
*/

#include <algorithm>
#include <cassert>
#include <chrono>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <format>
#include <experimental/simd> // mostly for immintrin.h
#include <numeric>
#include <print>
#include <random>
#include <vector>

struct Benchmark
{
  std::uint64_t const min;
  double const mean;
  std::uint64_t const median;
  double const stdDev;
  std::uint64_t const p90;
  std::uint64_t const p99;
  std::uint64_t const max;
};

template <>
struct std::formatter<Benchmark> 
{
  constexpr auto parse(std::format_parse_context& ctx) {
      return ctx.begin();
  }

  auto format(const Benchmark &b, std::format_context& ctx) const {
      return std::format_to(ctx.out(), "====== Benchmark results ======\n"
      "min {}ms\n"
      "mean {}ms\n"
      "median {}ms\n"
      "std dev {}ms\n"
      "p90 {}ms\n"
      "p99 {}ms\n"
      "max {}ms\n"
      "===============================", b.min, b.mean, b.median, b.stdDev, b.p90, b.p99, b.max);
  }
};

inline void doNotOptimize(auto const &value)
{ asm volatile("" : : "r,m"(value) : "memory"); }

auto measure(int nIter, std::invocable auto f) -> Benchmark
{
  auto timingResults = std::vector<std::uint64_t>();
  timingResults.reserve(nIter);

  for (int iter = 0; iter != nIter; ++iter)
  {
    auto const start = std::chrono::high_resolution_clock::now();

    auto value = f();
    doNotOptimize(value);

    auto const end = std::chrono::high_resolution_clock::now();
    auto const elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
    timingResults.push_back(elapsedTime);
  }

  std::ranges::sort(timingResults);
  double const mean = std::accumulate(timingResults.begin(), timingResults.end(), 0.)/nIter;
  
  double const sqSum = std::accumulate(timingResults.begin(), timingResults.end(), 0., 
      [mean](double acc, double x) { return acc + (x - mean) * (x - mean); });
  double const stdDev = std::sqrt(sqSum / nIter);

  auto getPercentile = [&](double p) {
      std::size_t const idx = static_cast<std::size_t>(std::ceil((p / 100.0) * nIter)) - 1;
      return timingResults[std::min(idx, std::size_t(nIter) - 1)];
  };

  return {timingResults.front(), mean, getPercentile(50), stdDev, getPercentile(90), getPercentile(99), timingResults.back()};
}

int main()
{
  auto constexpr vectorSize = 100'000'000;
  auto const data = std::vector<int>(vectorSize, 1);
  auto const nIter = 10;
  
  {
    auto const b = measure(nIter, [&](){
      decltype(data)::value_type sum = 0;
      for (auto const &e : data) sum += e;
      return sum;
    });
    std::println("Range-based timing results:");
    std::println("{}\n\n\n", b);
  }

  auto indices = std::vector<std::size_t>(vectorSize);
  std::ranges::iota(indices,0);

  {
    auto const b = measure(nIter, [&](){
      decltype(data)::value_type sum = 0;
      for (auto i = 0; i != vectorSize; ++i) sum += data[indices[i]];
      return sum;
    });
    std::println("Index indirection timing results");
    std::println("{}\n\n\n", b);
  }

  std::ranges::shuffle(indices, std::mt19937{std::random_device{}()});

  {
    auto const b = measure(nIter, [&](){
      decltype(data)::value_type sum = 0;
      for (auto i = 0; i != vectorSize; ++i) sum += data[indices[i]];
      return sum;
    });
    std::println("Shuffled indices timing results (no prefetch)");
    std::println("{}\n\n\n", b);
  }

  {
    for (auto prefetchDistance : std::array{1, 10, 50, 90, 100, 200, 300, 400, 500, 1000, 10000})
    { 
      auto const b = measure(nIter, [&](){
        decltype(data)::value_type sum = 0;
        
        for (auto i = 0; i != vectorSize; ++i) 
        {
          __builtin_prefetch(data.data()+indices[i+prefetchDistance]);
          sum += data[indices[i]];
        }
        return sum;
      });

      std::println("Prefetch Distance {} walltime is {}ms", prefetchDistance, b.mean);
    }
  }

#ifdef __AVX512F__
  auto constexpr lanes = std::experimental::native_simd<int>::size();
  
  {
    auto const b = measure(nIter, [&](){
        auto vecSum = _mm512_setzero_si512();

        std::size_t i = 0;
        for (; i + lanes <= vectorSize; i+=lanes)
        {
          auto const localIndices = _mm512_loadu_si512((__m512i*)&indices[i]);
          auto const vals = _mm512_i32gather_epi32(localIndices, data.data(), sizeof(decltype(data)::value_type));
          vecSum = _mm512_add_epi64(vecSum, vals);
        }

        auto const sum = _mm512_reduce_add_epi64(vecSum);

        return sum;
      });

    std::println("SIMD gather timing results");
    std::println("{}", b);
  }

  {
    for (auto prefetchDistance : std::array{1, 10, 50, 90, 100, 200, 300, 400, 500, 1000, 10000})
    {
      auto const b = measure(nIter, [&](){

        auto vecSum = _mm512_setzero_si512();

        std::size_t i = 0;
        for (; i + lanes <= vectorSize; i+=lanes)
        {
          for (auto iP = i+prefetchDistance; iP < std::min<std::size_t>(i+prefetchDistance+lanes, vectorSize); ++iP )
            __builtin_prefetch(data.data()+indices[iP]);

          auto const localIndices = _mm512_loadu_si512((__m512i*)&indices[i]);
          auto const vals = _mm512_i32gather_epi32(localIndices, data.data(), sizeof(decltype(data)::value_type));
          vecSum = _mm512_add_epi32(vecSum, vals);
        }

        auto const sum = _mm512_reduce_add_epi32(vecSum);

        return sum;
      });

      std::println("SIMD gather Prefetch Distance {} walltime is {}", prefetchDistance, b.mean);
    }
  }
#endif
}