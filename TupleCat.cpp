#include<tuple>

//Alternative tuple_cat implementations

auto constexpr tuple1 = std::tuple{42, 'c'};
auto constexpr tuple2 = std::tuple{'a',12356.,'d',4};
auto constexpr tuple3 = std::tuple{999};

template<typename T> //use tuple_like concept
using Sq = std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<T>>>;

auto constexpr zipper = []<auto ...I, auto ...J>(auto &&tuple1, auto &&tuple2, 
                                                 std::index_sequence<I...>, 
                                                 std::index_sequence<J...>)
{
  using tpl1_t = decltype(tuple1);
  using tpl2_t = decltype(tuple2);
  return std::tuple{std::get<I>(std::forward<tpl1_t>(tuple1))..., std::get<J>(std::forward<tpl2_t>(tuple2))...};
};

#ifdef LAMBDAHELPER
auto constexpr tuple_cat_helper = [](this auto &&self, auto &&tpl1, auto &&tpl2, auto &&...tpls)
{
  if constexpr (sizeof...(tpls) == 0) 
  {
    using tpl1_t = decltype(tpl1);
    using tpl2_t = decltype(tpl2);
    return zipper(std::forward<tpl1_t>(tpl1), std::forward<tpl2_t>(tpl2), Sq<tpl1_t>{}, Sq<tpl2_t>{});
  }
  else
  {
    using tpl1_t = decltype(tpl1);
    using tpl2_t = decltype(tpl2);
    return self(zipper(std::forward<tpl1_t>(tpl1), std::forward<tpl2_t>(tpl2), Sq<tpl1_t>{}, Sq<tpl2_t>{}), tpls...);
  }
};
#else
template<typename Tpl1>
auto constexpr tuple_cat_helper(Tpl1 &&tpl1)
{ return tpl1; }

template<typename Tpl1, typename Tpl2>
auto constexpr tuple_cat_helper(Tpl1 &&tpl1, Tpl2 &&tpl2)
{ 
  return zipper(std::forward<Tpl1>(tpl1), std::forward<Tpl2>(tpl2), Sq<Tpl1>{}, Sq<Tpl2>{}); 
}

template<typename Tpl1, typename Tpl2, typename ...Tpls>
auto constexpr tuple_cat_helper(Tpl1 &&tpl1, Tpl2 &&tpl2, Tpls &&...tpls)
{ return tuple_cat_helper(tuple_cat_helper(std::forward<Tpl1>(tpl1), std::forward<Tpl2>(tpl2)), std::forward<Tpls>(tpls)...); }
#endif

auto constexpr tuple_cat_local(auto &&...tpls)
{ return tuple_cat_helper(tpls...); }


static auto constexpr tupleCat = tuple_cat_local(tuple1, tuple3, tuple2, std::tuple{888});
static_assert(std::get<std::tuple_size_v<decltype(tupleCat)>-1>(tupleCat) == 888);

int main() {}
