#pragma once

#include "Direction.h"

template<Dir dir>
struct Index;

template<>
struct Index<X>
{
  inline auto operator+(int n) noexcept
  { return Index<X>{_i+n, _j, _k}; }

  int _i{};
  int _j{};
  int _k{};
};

template<>
struct Index<Y>
{
  inline auto operator+(int n) noexcept
  { return Index<Y>{_i, _j+n, _k}; }

  int _i{};
  int _j{};
  int _k{};
};

template<>
struct Index<Z>
{
  inline auto operator+(int n) noexcept
  { return Index<Z>{_i, _j, _k+n}; }

  int _i{};
  int _j{};
  int _k{};
};