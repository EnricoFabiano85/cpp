#include <new>
#include <vector>
#include <random>

#include "/home/enrico/Documents/coding/benchmark/include/benchmark/benchmark.h"

/*
g++ -std=c++23 -O3 benchmarkFDM.cpp -lbenchmark -lpfm
sudo ./a.out --benchmark_perf_counters=BRANCH-MISSES,CACHE-MISSES,CACHE-REFERENCES --benchmark_counters_tabular=true
*/

namespace AlignedAllocatorConcepts
{
  template<std::size_t ALIGNMENT, std::size_t SIZE>
  concept SizeIsMultiple = (SIZE%ALIGNMENT == 0);

  template<typename T, std::size_t ALIGNMENT>
  concept MinAlignment = (ALIGNMENT >= alignof(T));
}

// Custom allocator adapted from
// https://stackoverflow.com/a/70994249
template<typename T, std::size_t ALIGNMENT>
requires AlignedAllocatorConcepts::MinAlignment<T, ALIGNMENT>
class AlignedAllocator
{
public:
    using value_type = T;

    /**
     * This is only necessary because AlignedAllocator has a second template
     * argument for the alignment that will make the default
     * std::allocator_traits implementation fail during compilation.
     * @see https://stackoverflow.com/a/48062758/2191065
     */
    template<class U>
    struct rebind { using other = AlignedAllocator<U, ALIGNMENT>; };

public:
    constexpr AlignedAllocator() noexcept = default;

    constexpr AlignedAllocator( const AlignedAllocator& ) noexcept = default;

    template<typename U>
    constexpr AlignedAllocator( AlignedAllocator<U, ALIGNMENT> const& ) noexcept {}

    [[nodiscard]] 
    T* allocate( std::size_t nElementsToAllocate )
    {
        if ( nElementsToAllocate > std::numeric_limits<std::size_t>::max() / sizeof(T))
          throw std::bad_array_new_length();

        auto const nBytesToAllocate = nElementsToAllocate * sizeof(T);
        void *ptr = std::aligned_alloc(ALIGNMENT, nBytesToAllocate);
        if (!ptr) throw std::bad_alloc{};

        return reinterpret_cast<T*>(ptr);
    }

    void
    deallocate(T* allocatedPointer, [[maybe_unused]] std::size_t  nBytesAllocated )
    { std::free(allocatedPointer); }
};


struct Solution
{
  double rho;
  double rhoU;
  double rhoV;
  double rhoW;
  double rhoE;
};

void BM_FdmScalar(benchmark::State& state)
{
  auto const vectorSize = state.range(0);
  std::vector<double> vec(vectorSize);
  std::random_device rd{};
  std::mt19937 gen{rd()};
  std::normal_distribution d{10.0, 2.0};
  auto random = [&d, &gen](){ return d(gen); };
  std::generate(vec.begin(), vec.end(), random);

  double sum = 0;
  for (auto _ : state)
  {
    for (auto idx = 1; idx < vectorSize-1; ++idx)
      sum = vec[idx-1] + vec[idx] + vec[idx-1];

    benchmark::DoNotOptimize(sum);
    benchmark::ClobberMemory();
    // state.SetItemsProcessed(vectorSize*state.iterations());
  }
}

void BM_FdmBlock(benchmark::State& state)
{
  auto const vectorSize = state.range(0);
  std::vector<Solution> vec(vectorSize);
  std::random_device rd{};
  std::mt19937 gen{rd()};
  std::normal_distribution d{10.0, 2.0};
  auto random = [&d, &gen](){ return Solution{d(gen), d(gen), d(gen), d(gen), d(gen)}; };
  std::generate(vec.begin(), vec.end(), random);

  double sum = 0;
  for (auto _ : state)
  {
    for (auto idx = 1; idx < vectorSize-1; ++idx)
      sum = vec[idx-1].rho + vec[idx].rho + vec[idx-1].rho;

    benchmark::DoNotOptimize(sum);
    benchmark::ClobberMemory();
    // state.SetItemsProcessed(vectorSize*state.iterations());
  }
}

void BM_FdmBlockAligned(benchmark::State& state)
{
  auto const vectorSize = state.range(0);
  auto constexpr alignment = std::hardware_destructive_interference_size;
  std::vector<Solution, AlignedAllocator<Solution, alignment>> vec(vectorSize);
  std::random_device rd{};
  std::mt19937 gen{rd()};
  std::normal_distribution d{10.0, 2.0};
  auto random = [&d, &gen](){ return Solution{d(gen), d(gen), d(gen), d(gen), d(gen)}; };
  std::generate(vec.begin(), vec.end(), random);

  double sum = 0;
  for (auto _ : state)
  {
    for (auto idx = 1; idx < vectorSize-1; ++idx)
      sum = vec[idx-1].rho + vec[idx].rho + vec[idx-1].rho;

    benchmark::DoNotOptimize(sum);
    benchmark::ClobberMemory();
    // state.SetItemsProcessed(vectorSize*state.iterations());
  }
}

#define ARGS \
    ->DenseRange(10,20000,500);

BENCHMARK(BM_FdmScalar)->Arg(100);
BENCHMARK(BM_FdmBlock)->Arg(100);
BENCHMARK(BM_FdmBlockAligned)->Arg(100);

BENCHMARK_MAIN();

