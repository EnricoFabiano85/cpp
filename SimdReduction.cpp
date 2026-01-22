#include <chrono>
#include <experimental/simd>
#include <numeric>
#include <print>
#include <span>
#include <vector>

namespace stdx = std::experimental;

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
  for ( ; i+V::size() < data.size(); i += V::size())
    //needs 64-bytes aligned memory to use stdx::vector_aligned with avx512f
    //alternatively use stdx::element_aligned
    acc += stdx::reduce(V(&data[i], stdx::vector_aligned), std::plus{});

  for (; i < data.size(); ++i) acc += data[i];

  return acc;
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
  using V = stdx::native_simd<double>;

  auto constexpr aligned = true;
  using Allocator = std::conditional_t<aligned, AlignedAllocator<V::value_type, 64>, std::allocator<V::value_type>>;

  auto data = std::vector<V::value_type, Allocator>(1<<30);
  std::ranges::iota(data, 0);
  auto const span = std::span(data);

  __builtin___clear_cache(reinterpret_cast<char*>(span.data()), reinterpret_cast<char*>(span.data()+span.size()));

  auto const resSimd = measure([&](){ return simdSum(span);});
  std::println("simd sum   = {} {}", resSimd.first, resSimd.second);

  __builtin___clear_cache(reinterpret_cast<char*>(span.data()), reinterpret_cast<char*>(span.data()+span.size()));

  auto resLinear = measure([&](){ return linearSum(span);});
  std::println("linear sum = {} {}", resLinear.first, resLinear.second);
}