#include<iostream>
#include<array>
#include<numeric>

/*
Two answers to
https://www.hackerrank.com/challenges/cpp-variadics/problem?isFullScreen=true
(for the part of the question that I understand)
*/

template<int... I>
consteval auto reverseBinary()
{
  std::array<int, sizeof...(I)> data{I...};

  auto powerInt = [](this auto&& self, int x, int p){
    if (p == 0) return 1;
    if (p == 1) return x;
    
    int tmp = self(x, p/2);
    if (p%2 == 0) return tmp * tmp;
    else return x * tmp * tmp;
  };

  int index = sizeof...(I);
  int decimal = std::accumulate(data.crbegin(), data.crend(), 0,
  [&](int a, int b){
    --index;
    return a + b*powerInt(2,index);
  });

  return decimal;
}

template<int... I, int... J>
constexpr auto reverseBinaryFoldImpl(std::integer_sequence<int, J...> index)
{
  auto powerInt = [](this auto&& self, int x, int p){
    if (p == 0) return 1;
    if (p == 1) return x;
    
    int tmp = self(x, p/2);
    if (p%2 == 0) return tmp * tmp;
    else return x * tmp * tmp;
  };
  
  return ((I*powerInt(2,J))+...);
}

template<int... I>
constexpr auto reverseBinaryFold()
{ return reverseBinaryFoldImpl<I...>(std::make_integer_sequence<int, sizeof...(I)>{}); }

static_assert(reverseBinary<0,0,1>()==4);
static_assert(reverseBinaryFold<0,0,1,1>()==12);

int main()
{}