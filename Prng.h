#include <array>
#include <charconv>
#include <cstdint>

/*
  C++23 compile-time pseudo random number generator inspired from
  https://mklimenko.github.io/english/2018/06/04/constexpr-random/

  LCE parameters from: https://en.cppreference.com/w/cpp/numeric/random/linear_congruential_engine
*/

struct Prng
{
  using UIntF32 = std::uint_fast32_t;
  static constexpr UIntF32 _a = 48271;
  static constexpr UIntF32 _c = 0;
  static constexpr UIntF32 _m = 2147483647;

  static consteval auto getSeed()
  {
    auto time = __TIME__;
    UIntF32 hours{}, mins{}, secs{};
    std::from_chars(time+0, time+2, hours);
    std::from_chars(time+3, time+5, mins);
    std::from_chars(time+6, time+8, secs);

    return hours + mins + secs;
  }

  template<std::size_t N>
  static consteval auto uniformDistribution(double const min, double const max)
  {
    auto distribution = std::array<double, N>();
    auto rand = getSeed();

    for (auto &element : distribution)
    {
      rand = ((_a * rand + _c) % _m);
      double randDouble = static_cast<double>(rand) / _m;
      element = randDouble*(max-min)+min;
    }

    return distribution;
  }
};