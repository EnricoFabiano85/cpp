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
#include <cstddef>
#include <experimental/simd> // mostly for immintrin.h
#include <print>
#include <random>
#include <vector>

int main()
{
  auto constexpr vectorSize = 100000000;
  auto const data = std::vector<int>(vectorSize, 1);
  
  {
    auto sum = 0;

    auto const start = std::chrono::high_resolution_clock::now();

    for (auto const &e : data) sum += e;

    auto const end = std::chrono::high_resolution_clock::now();
    auto const elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();

    assert(sum == vectorSize);
    std::println("Range-based for walltime is {}ms", elapsedTime);
  }

  auto indices = std::vector<std::size_t>(vectorSize);
  std::ranges::iota(indices,0);

  {
    auto sum = 0;

    auto const start = std::chrono::high_resolution_clock::now();

    for (auto i = 0; i < vectorSize; i++) sum += data[indices[i]];

    auto const end = std::chrono::high_resolution_clock::now();
    auto const elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();

    assert(sum == vectorSize);
    std::println("Index indirection walltime is {}ms", elapsedTime);
  }

  std::ranges::shuffle(indices, std::mt19937{std::random_device{}()});

  {
    auto sum  = 0;

    auto const start = std::chrono::high_resolution_clock::now();

    for (auto i = 0; i < vectorSize; i++) sum += data[indices[i]];

    auto const end = std::chrono::high_resolution_clock::now();
    auto const elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();

    assert(sum == vectorSize);
    std::println("Shuffled indices (no prefetch) walltime is {}ms", elapsedTime);
  }

  {
    for (auto prefetchDistance : std::array{1, 10, 50, 90, 100, 200, 300, 400, 500, 1000, 10000})
    {  
      auto sum = 0;

      auto const startPrefetch = std::chrono::high_resolution_clock::now();

      for (auto i = 0; i < vectorSize; i++) 
      {
        __builtin_prefetch(data.data()+indices[i+prefetchDistance]);
        sum += data[indices[i]];
      }

      auto const endPrefetch = std::chrono::high_resolution_clock::now();
      auto const elapsedPrefetch = std::chrono::duration_cast<std::chrono::milliseconds>(endPrefetch-startPrefetch).count();

      assert(sum == vectorSize);
      std::println("Prefetch Distance {} walltime is {}ms", prefetchDistance, elapsedPrefetch);
    }
  }

#ifdef __AVX512F__
  auto constexpr lanes = std::experimental::native_simd<int>::size();
  
  {
    auto const startSimd = std::chrono::high_resolution_clock::now();

    auto vecSum = _mm512_setzero_si512();

    std::size_t i = 0;
    for (; i + lanes <= vectorSize; i+=lanes)
    {
      auto const localIndices = _mm512_loadu_si512((__m512i*)&indices[i]);
      auto const vals = _mm512_i32gather_epi32(localIndices, data.data(), sizeof(decltype(data)::value_type));
      vecSum = _mm512_add_epi32(vecSum, vals);
    }

    auto const sum = _mm512_reduce_add_epi32(vecSum);

    auto const endSimd = std::chrono::high_resolution_clock::now();
    auto const elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endSimd-startSimd).count();

    assert(sum == vectorSize);
    std::println("SIMD gather walltime is {}ms", elapsedTime);
  }

  {
    for (auto prefetchDistance : std::array{1, 10, 50, 90, 100, 200, 300, 400, 500, 1000, 10000})
    {
      auto const startSimdPrefetch = std::chrono::high_resolution_clock::now();

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

      auto const endSimdPrefetch = std::chrono::high_resolution_clock::now();
      auto const elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endSimdPrefetch-startSimdPrefetch).count();

      assert(sum == vectorSize);
      std::println("SIMD gather Prefetch Distance {} walltime is {}ms", prefetchDistance, elapsedTime);
    }
  }
#endif
}