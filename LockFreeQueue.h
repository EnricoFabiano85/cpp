#pragma once
#include <array>
#include <atomic>
#include <bit>
#include <concepts>
#include <optional>

/*
Circular array implementation of single-producer, single-consumer lock-free queue.
*/

namespace LFQueueConstraints
{
  template<std::size_t N>
  concept PowerTwo = std::has_single_bit(N);
}

template<typename T, std::size_t N> 
requires (std::default_initializable<T> && std::movable<T> && LFQueueConstraints::PowerTwo<N>)
class LockFreeQueue 
{
public:
  LockFreeQueue() = default;

  void push(T &&e) noexcept
  {
    auto const write = _write.load(std::memory_order_relaxed);

    while (write == _read.load(std::memory_order_acquire) + N) std::this_thread::yield(); //__mm_pause();

    _data[getArrayIndex(write)] = std::move(e);

    _write.store(write + 1, std::memory_order_release);
  }

  std::optional<T> pop() noexcept
  {
    auto res = std::optional<T>{};

    auto const read = _read.load(std::memory_order_relaxed);
    auto const write = _write.load(std::memory_order_acquire);

    if (write != read)
    {
      res = std::move(_data[getArrayIndex(read)]);
      _read.store(read + 1, std::memory_order_release);
    }

    return res;
  }

  size_t size() const noexcept 
  {
    auto const w = _write.load(std::memory_order_relaxed);
    auto const r = _read.load(std::memory_order_relaxed);
    
    return w - r; 
  }

private:

  std::size_t inline getArrayIndex(std::size_t const index) const noexcept
  { 
    // https://stackoverflow.com/a/19383296
    return (index & (N - 1)); 
  }

  alignas(64) std::atomic<std::size_t> _read{};
  alignas(64) std::atomic<std::size_t> _write{};
  std::array<T, N> _data{};
};