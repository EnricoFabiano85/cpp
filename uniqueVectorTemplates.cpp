#include <algorithm>
#include <array>
#include <iostream>
#include <utility>

// Assignement from https://www.mavensecurities.com/revisiting-interview-questions-at-maven/
// Implementation slightly different from C++20 solution there

template <int... I>
struct Vector {};
 
template <typename Vector>
struct Unique;

template<int... I> struct Unique<Vector<I...>> 
{
  static auto consteval uniqueValues()
  {
    struct { std::array<int, sizeof...(I)> data; int size; } r{{I...}, sizeof...(I)};
    r.size = std::unique(r.data.begin(), r.data.end()) - r.data.begin();
    return r;
  }

  template<std::array<int, sizeof...(I)> array, int... J>
  static auto consteval getVector_impl(std::integer_sequence<int, J...>)
  {
    return Vector<array[J]...>();      
  }

  static auto consteval getVector()
  {
    auto constexpr uniqueVector = uniqueValues();
    return getVector_impl<uniqueVector.data>(std::make_integer_sequence<int, uniqueVector.size>{});
  }

  using type = decltype(getVector());
};

static_assert(std::is_same_v<Unique<Vector<1, 2, 2, 2, 3, 4, 4, 1, 5>>::type, Vector<1, 2, 3, 4, 1, 5>>);


int main()
{}
