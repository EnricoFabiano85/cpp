#include <tuple>
#include <utility>
#include <type_traits>

namespace ModernTuple::details
{
constexpr auto 
makeTuple(auto&& ...args)
{
  return [...args = std::forward<decltype(args)>(args)]<std::size_t I>
          -> decltype(auto)
  {
    return args...[I];
  };
}

template<typename T>
using Sq = std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<T>>>;
}


namespace ModernTuple {
template<typename ...T>
struct Tuple
{
  constexpr Tuple(auto&& ...args) : _lambdaTuple(details::makeTuple(std::forward<decltype(args)>(args)...))
  {}

  template<typename ...Args>
  constexpr Tuple(Tuple<Args...> const &other) : _lambdaTuple([other]<auto ...I>(std::index_sequence<I...>){
                                                              return details::makeTuple(get<I>(other)...);
                                                              }(std::make_index_sequence<sizeof...(Args)>{}))
  {}

  template<typename ...Args>
  constexpr Tuple(Tuple<Args...> &&other) : _lambdaTuple([other]<auto ...I>(std::index_sequence<I...>){
                                                         return details::makeTuple(std::get<I>(std::move(other))...);
                                                         }(std::make_index_sequence<sizeof...(Args)>{}))
  {}

  decltype(details::makeTuple(std::declval<T>()...)) _lambdaTuple;
};

Tuple(auto&& ...args) -> Tuple<std::decay_t<decltype(args)>...>;

template<typename ...Args>
Tuple(Tuple<Args...> const &other) -> Tuple<Args...>;

namespace details
{

template<typename T>
struct is_modern_tuple_helper : std::bool_constant<false> {};

template<typename ...Types>
struct is_modern_tuple_helper<ModernTuple::Tuple<Types...>> : std::bool_constant<true> {};

template<typename Tuple>
concept is_modern_tuple = is_modern_tuple_helper<std::decay_t<Tuple>>::value;

}

template<std::size_t I, typename T>
struct tuple_element;

template<std::size_t I, typename ...Elem>
struct tuple_element<I, Tuple<Elem...>>
{
  using type = Elem...[I];
};

template<auto I>
decltype(auto) constexpr get(ModernTuple::details::is_modern_tuple auto &&t) noexcept
{
  return std::forward<decltype(t)>(t)._lambdaTuple.template operator()<I>();
}

template<typename Tpl1>
auto constexpr tuple_cat(Tpl1 &&tpl1)
{ return tpl1; }

template<typename Tpl1, typename Tpl2>
auto constexpr tuple_cat(Tpl1 &&tpl1, Tpl2 &&tpl2)
{ 
  return []<auto ...I, auto ...J>(auto &&tuple1, auto &&tuple2, std::index_sequence<I...>, std::index_sequence<J...>) 
  { return Tuple{std::get<I>(tuple1)..., std::get<J>(tuple2)...}; }
  (std::forward<Tpl1>(tpl1), std::forward<Tpl2>(tpl2), details::Sq<Tpl1>{}, details::Sq<Tpl2>{}); 
}

template<typename Tpl1, typename Tpl2, typename ...Tpls>
auto constexpr tuple_cat(Tpl1 &&tpl1, Tpl2 &&tpl2, Tpls &&...tpls)
{ return tuple_cat(tuple_cat(std::forward<Tpl1>(tpl1), std::forward<Tpl2>(tpl2)), std::forward<Tpls>(tpls)...); }
}


namespace std {
template<size_t I, typename ...Elem>
struct tuple_element<I, ModernTuple::Tuple<Elem...>> : ModernTuple::tuple_element<I, ModernTuple::Tuple<Elem...>> {};

template<std::size_t I, typename... Elem>
struct tuple_element<I, ModernTuple::Tuple<Elem...> const> 
{
  using type = add_const_t<typename tuple_element<I, ModernTuple::Tuple<Elem...>>::type>;
};

template<std::size_t I, typename Tuple>
struct tuple_element<I, Tuple&> : tuple_element<I, Tuple> {};

template<std::size_t I, typename Tuple>
struct tuple_element<I, Tuple&&> : tuple_element<I, Tuple> {};

template<typename ...Elem>
struct tuple_size<ModernTuple::Tuple<Elem...>> : std::integral_constant<std::size_t, sizeof...(Elem)>{};

template<typename Tuple>
struct tuple_size<Tuple &> : tuple_size<Tuple> {};

template<typename Tuple>
struct tuple_size<Tuple &&> : tuple_size<Tuple> {};
}

#ifdef ENABLE_TESTS
auto constexpr array1 = std::tuple{42, 'c'};
auto constexpr array2 = std::tuple{'a',12356.,'d',4};
auto constexpr array3 = std::tuple{999};

static auto constexpr tupleCat = std::tuple_cat(array1, array3, array2, std::tuple{888});
static_assert(std::get<std::tuple_size_v<decltype(tupleCat)>-1>(tupleCat) == 888);
#endif