#include <array>
#include <tuple>

template<int ...I>
struct Vector
{
  static constexpr std::array<int, sizeof...(I)> _data{I...};
  static constexpr std::size_t size = sizeof...(I);
};

template<typename ...V>
struct Zip
{
  static constexpr std::size_t vectorSize{std::tuple_element_t<0, std::tuple<V...>>::size};

  static consteval auto getZippedVector()
  {
    auto constexpr array = [](auto ...v)
                           {
                             auto result = std::array<int, vectorSize>{};
                             result.fill(1);
                             for (std::size_t I = 0; I < vectorSize; ++I)
                               result[I] *= (decltype(v)::_data[I] * ...);
                             
                             return result;
                           }(V{}...);
    
    return [array]<std::size_t ...I>(std::index_sequence<I...>)
     { return Vector<array[I]...>{}; }(std::make_index_sequence<vectorSize>{});
  }

  using type = decltype(getZippedVector());
};

using Test = Zip<Vector<1,2,3>, 
                 Vector<2,3,4>, 
                 Vector<3,4,5>>::type;
using Expected = Vector<6,24,60>;

static_assert(std::same_as<Test, Expected>);

int main()
{
  return 0;
}