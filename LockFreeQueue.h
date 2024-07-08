#pragma once
#include <array>
#include <atomic>
#include <bit>

/*
  WIP: Circular array implementation of lock-free queue.
*/

namespace LFQueueConstraints
{
  template<std::size_t N>
  concept PowerTwo = ((N & (N - 1)) == 0);

  template<typename T>
  concept DefaultConstructible = std::is_default_constructible_v<T>;
}

template<LFQueueConstraints::DefaultConstructible T, std::size_t N> requires LFQueueConstraints::PowerTwo<N>
class LockFreeQueue 
{
public:
  LockFreeQueue() = default;

  void push(T &&element)
  {
    _data[getArrayIndex(_write)] = std::move(element);
    ++_write;
    _size.fetch_add(1, std::memory_order_release);
  }

  auto pop()
  {
    return std::move(_data[getArrayIndex(_read)]);
    ++_read;
    _size.fetch_sub(1, std::memory_order_release);
  }
private:

  auto getArrayIndex(std::size_t const index) const
  { 
    // https://stackoverflow.com/a/19383296
    return (index & (N - 1)); 
  }

  std::size_t _read{};
  std::size_t _write{};
  std::atomic<std::size_t> _size{};
  std::array<T, N> _data{};
};