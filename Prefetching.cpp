#include <algorithm>
#include <cassert>
#include <chrono>
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
    std::println("Range-based for walltime is {}", elapsedTime);
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
    std::println("Index indirection walltime is {}", elapsedTime);
  }

  std::ranges::shuffle(indices, std::mt19937{std::random_device{}()});

  {
    auto sum  = 0;

    auto const start = std::chrono::high_resolution_clock::now();

    for (auto i = 0; i < vectorSize; i++) sum += data[indices[i]];

    auto const end = std::chrono::high_resolution_clock::now();
    auto const elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();

    assert(sum == vectorSize);
    std::println("Shuffled indices (no prefetch) walltime is {}", elapsedTime);
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
      std::println("Prefetch Distance {} walltime is {}", prefetchDistance, elapsedPrefetch);
    }
  }
}