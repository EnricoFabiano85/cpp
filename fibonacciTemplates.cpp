/*
  Compute fibonacci sequence at compile time (two approaches, one needs c++23)
*/
#include <array>
#include <type_traits>


template<int N>
using Fibonacci = std::integral_constant<int, [memo = std::array<int, N>{}](this auto&&self, int K){
    if (K <= 1)
      return K;
    else
    {
      if (memo[K-1] == 0)
      {
        memo[K-1] = self(K-1) + self(K-2);
      }

      return memo[K-1];
    }
}(N)>;

template<int N>
struct Fibonacci2
{
  static int constexpr value = Fibonacci2<N-1>::value + Fibonacci2<N-2>::value;
};

template<>
struct Fibonacci2<1> {static int constexpr value = 1;};

template<>
struct Fibonacci2<0> {static int constexpr value = 0;};

static_assert(Fibonacci<40>::value == 102334155);
static_assert(Fibonacci2<10>::value == 55);

int main()
{}