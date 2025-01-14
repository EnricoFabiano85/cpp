#pragma once
#include <array>
#include <atomic>
#include <bit>
#include <optional>

/*
Circular array implementation of single-producer, single-consumer lock-free queue.
*/

namespace LFQueueConstraints
{
  // template<std::size_t N>
  // concept PowerTwo = ((N & (N - 1)) == 0);

  template<std::size_t N>
  concept PowerTwo = std::has_single_bit(N);

  template<typename T>
  concept DefaultConstructible = std::is_default_constructible_v<T>;
}

template<typename T, std::size_t N> 
requires (LFQueueConstraints::DefaultConstructible<T> && LFQueueConstraints::PowerTwo<N>)
class LockFreeQueue 
{
public:
  LockFreeQueue() = default;

  void push(T &&e) noexcept
  {
    _data[getArrayIndex(_write)] = std::move(e);
    ++_write;
    _size.fetch_add(1, std::memory_order_release);
  }

  auto pop() noexcept
  {
    if (_size != 0)
    {
      auto res = std::move(_data[getArrayIndex(_read)]);
      ++_read;
      _size.fetch_sub(1, std::memory_order_release);
      return std::optional{res};
    }
    else 
    {
      return std::optional<T>(std::nullopt); 
    }
  }

  bool inline empty() const noexcept
  {
    return (_size==0);
  }

private:

  std::size_t inline getArrayIndex(std::size_t const index) const noexcept
  { 
    // https://stackoverflow.com/a/19383296
    return (index & (N - 1)); 
  }

  std::size_t _read{};
  std::size_t _write{};
  std::atomic<std::size_t> _size{};
  std::array<T, N> _data{};
};