#include <new>
#include <ranges>
#include <vector>
#include <random>

#include "/home/enrico/Documents/coding/benchmark/include/benchmark/benchmark.h"

/*
  g++ -std=c++23 -O3 benchmarkFDM.cpp -lbenchmark -lpfm
  sudo ./a.out --benchmark_perf_counters=BRANCH-MISSES,CACHE-MISSES,CACHE-REFERENCES --benchmark_counters_tabular=true
  See https://stackoverflow.com/posts/77091177/revisions to interpret results from benchmark
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


void BM_FdmCentralScalar(benchmark::State& state)
{
  auto const vectorSize = state.range(0);
  std::vector<double> rho(vectorSize);
  std::vector<double> rhoU(vectorSize);
  std::vector<double> rhoV(vectorSize);
  std::vector<double> rhoW(vectorSize);
  std::vector<double> rhoE(vectorSize);
  std::random_device rd{};
  std::mt19937 gen{rd()};
  std::normal_distribution d{10.0, 2.0};
  auto random = [&d, &gen](){ return d(gen); };
  std::ranges::generate(rho, random);

  double sum = 0;
  for (auto _ : state)
  {
    for (auto idx = 1; idx < vectorSize-1; ++idx)
    {
      sum = rho[idx-1] + rho[idx] + rho[idx+1];
      sum += rhoU[idx-1] + rhoU[idx] + rhoU[idx+1];
      sum += rhoV[idx-1] + rhoV[idx] + rhoV[idx+1];
      sum += rhoW[idx-1] + rhoW[idx] + rhoW[idx+1];
      sum += rhoE[idx-1] + rhoE[idx] + rhoE[idx+1];
    }
      

    benchmark::DoNotOptimize(sum);
    benchmark::ClobberMemory();
    // state.SetItemsProcessed(vectorSize*state.iterations());
  }
}

void BM_FdmCentralBlock(benchmark::State& state)
{
  auto const vectorSize = state.range(0);
  std::vector<Solution> vec(vectorSize);
  std::random_device rd{};
  std::mt19937 gen{rd()};
  std::normal_distribution d{10.0, 2.0};
  auto random = [&d, &gen](){ return Solution{d(gen), d(gen), d(gen), d(gen), d(gen)}; };
  std::ranges::generate(vec, random);

  double sum = 0;
  for (auto _ : state)
  {
    for (auto idx = 1; idx < vectorSize-1; ++idx)
    {
      sum = vec[idx-1].rho + vec[idx].rho + vec[idx+1].rho;
      sum += vec[idx-1].rhoU + vec[idx].rhoU + vec[idx+1].rhoU;
      sum += vec[idx-1].rhoV + vec[idx].rhoV + vec[idx+1].rhoV;
      sum += vec[idx-1].rhoW + vec[idx].rhoW + vec[idx+1].rhoW;
      sum += vec[idx-1].rhoE + vec[idx].rhoE + vec[idx+1].rhoE;
    }

    benchmark::DoNotOptimize(sum);
    benchmark::ClobberMemory();
    // state.SetItemsProcessed(vectorSize*state.iterations());
  }
}

void BM_FdmCentralBlockAligned(benchmark::State& state)
{
  auto const vectorSize = state.range(0);
  auto constexpr alignment = std::hardware_destructive_interference_size;
  std::vector<Solution, AlignedAllocator<Solution, alignment>> vec(vectorSize);
  std::random_device rd{};
  std::mt19937 gen{rd()};
  std::normal_distribution d{10.0, 2.0};
  auto random = [&d, &gen](){ return Solution{d(gen), d(gen), d(gen), d(gen), d(gen)}; };
  std::ranges::generate(vec, random);

  double sum = 0;
  for (auto _ : state)
  {
    for (auto idx = 1; idx < vectorSize-1; ++idx)
    {
      sum = vec[idx-1].rho + vec[idx].rho + vec[idx+1].rho;
      sum += vec[idx-1].rhoU + vec[idx].rhoU + vec[idx+1].rhoU;
      sum += vec[idx-1].rhoV + vec[idx].rhoV + vec[idx+1].rhoV;
      sum += vec[idx-1].rhoW + vec[idx].rhoW + vec[idx+1].rhoW;
      sum += vec[idx-1].rhoE + vec[idx].rhoE + vec[idx+1].rhoE;
    }
      

    benchmark::DoNotOptimize(sum);
    benchmark::ClobberMemory();
    // state.SetItemsProcessed(vectorSize*state.iterations());
  }
}

void BM_FdmWenoBlock(benchmark::State& state)
{
  auto const vectorSize = state.range(0);
  std::vector<Solution> vec(vectorSize);
  std::random_device rd{};
  std::mt19937 gen{rd()};
  std::normal_distribution d{10.0, 2.0};
  auto random = [&d, &gen](){ return Solution{d(gen), d(gen), d(gen), d(gen), d(gen)}; };
  std::ranges::generate(vec, random);

  double sum = 0;
  for (auto _ : state)
  {
    for (auto idx = 2; idx < vectorSize-2; ++idx)
    {
      sum = vec[idx-2].rho + vec[idx-1].rho + vec[idx].rho + vec[idx+1].rho + vec[idx+2].rho;
      sum += vec[idx-2].rhoU + vec[idx-1].rhoU + vec[idx].rhoU + vec[idx+1].rhoU + vec[idx+2].rhoU;
      sum += vec[idx-2].rhoV + vec[idx-1].rhoV + vec[idx].rhoV + vec[idx+1].rhoV + vec[idx+2].rhoV;
      sum += vec[idx-2].rhoW + vec[idx-1].rhoW + vec[idx].rhoW + vec[idx+1].rhoW + vec[idx+2].rhoW;
      sum += vec[idx-2].rhoE + vec[idx-1].rhoE + vec[idx].rhoE + vec[idx+1].rhoE + vec[idx+2].rhoE;
    }

    benchmark::DoNotOptimize(sum);
    benchmark::ClobberMemory();
    // state.SetItemsProcessed(vectorSize*state.iterations());
  }
}

void BM_FdmWenoBlockAligned(benchmark::State& state)
{
  auto const vectorSize = state.range(0);
  auto constexpr alignment = std::hardware_destructive_interference_size;
  std::vector<Solution, AlignedAllocator<Solution, alignment>> vec(vectorSize);
  std::random_device rd{};
  std::mt19937 gen{rd()};
  std::normal_distribution d{10.0, 2.0};
  auto random = [&d, &gen](){ return Solution{d(gen), d(gen), d(gen), d(gen), d(gen)}; };
  std::ranges::generate(vec, random);

  double sum = 0;
  for (auto _ : state)
  {
    for (auto idx = 1; idx < vectorSize-1; ++idx)
    {
      sum = vec[idx-2].rho + vec[idx-1].rho + vec[idx].rho + vec[idx+1].rho + vec[idx+2].rho;
      sum += vec[idx-2].rhoU + vec[idx-1].rhoU + vec[idx].rhoU + vec[idx+1].rhoU + vec[idx+2].rhoU;
      sum += vec[idx-2].rhoV + vec[idx-1].rhoV + vec[idx].rhoV + vec[idx+1].rhoV + vec[idx+2].rhoV;
      sum += vec[idx-2].rhoW + vec[idx-1].rhoW + vec[idx].rhoW + vec[idx+1].rhoW + vec[idx+2].rhoW;
      sum += vec[idx-2].rhoE + vec[idx-1].rhoE + vec[idx].rhoE + vec[idx+1].rhoE + vec[idx+2].rhoE;
    }
      

    benchmark::DoNotOptimize(sum);
    benchmark::ClobberMemory();
    // state.SetItemsProcessed(vectorSize*state.iterations());
  }
}

#define ARGS \
    ->DenseRange(10,2000000,10000);

BENCHMARK(BM_FdmCentralScalar)->Arg(10000000);
BENCHMARK(BM_FdmCentralBlock)->Arg(10000000);
BENCHMARK(BM_FdmCentralBlockAligned)->Arg(10000000);
BENCHMARK(BM_FdmWenoBlock)->Arg(10000000);
BENCHMARK(BM_FdmWenoBlockAligned)->Arg(10000000);

BENCHMARK_MAIN();

