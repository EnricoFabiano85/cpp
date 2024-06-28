#include <array>

template<double X, int N>
struct Pow
{ static auto constexpr value = X*Pow<X, N-1>::value; };

template<double X>
struct Pow<X, 0>
{ static auto constexpr value = 1; };


template<double X, int N, std::size_t... Node>
auto constexpr polynomial_impl(std::index_sequence<Node...>, std::array<double, N> const coeff)
{ return ((coeff[Node]*Pow<X, Node>::value) + ...); }

template<double X, int N>
auto constexpr polynomial(std::array<double, N> const array)
{ return polynomial_impl<X, N>(std::make_index_sequence<N>{}, array); }


static_assert(Pow<2.0, 3>::value == 8.);
static_assert(polynomial<2.5, 3>(std::array<double, 3>{2., 4., -5.}) == -19.25);

int main()
{}