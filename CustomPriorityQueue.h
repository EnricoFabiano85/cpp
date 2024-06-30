#pragma once

#include <array>
#include <algorithm>
#include <functional>
#include <type_traits>

/* 
  Implementation of priority queue based on std::array
  with ability to:
  1) change priority of any of its element 
  2) remove random element from list elements
  (expensive operation)
*/

namespace QueueConstraints {

template<typename T>
concept DefaultConstructible = std::is_default_constructible_v<T>;

template<typename T>
concept Comparable = requires(T const &t1, T const &t2)
{
  std::is_default_constructible_v<T> && t1 < t2 && t1 > t2 && t1 == t2;
};

template<typename T>
concept DefaultConstructibleAndComparable = DefaultConstructible<T> && Comparable<T>;

template<typename Predicate, typename T>
concept Invocable = std::is_invocable_v<Predicate, T>;
}


template<QueueConstraints::DefaultConstructibleAndComparable T, int N=2048, typename Compare=std::less<T>>
class CustomPriorityQueue
{
public:

  CustomPriorityQueue() = default;

  template<typename U>
  void push(U &&element)
  {
    *_tail = std::move(element);
    ++_tail;
    std::push_heap(_data.begin(), _tail, _comp);
  }

  auto &top() const
  { return *_data.begin(); }

  void pop()
  {
    std::pop_heap(_data.begin(), _tail, _comp);
    --_tail;
  }

  auto size() const
  { return std::distance(_data.begin(), const_iterator(_tail)); }

  bool empty() const
  { return size() == 0; }

  template<typename U, QueueConstraints::Invocable<T> Predicate>
  void modify(U &&newData, Predicate &&pred)
  {
    if (auto element = std::find_if(_data.begin(), _tail, pred); element != std::end(_data))
    {
      *element = std::move(newData);
      std::make_heap(_data.begin(), _tail, _comp);
    }
  }

  template<typename Predicate>
  void remove(Predicate &&pred)
  {
    if (auto element = std::find_if(_data.begin(), _tail, pred); element != std::end(_data))
    {
      std::swap(*element, *_tail);
      --_tail;
      std::make_heap(_data.begin(), _tail, _comp);
    }
  }

private:
  using Container = std::array<T, N>;
  using iterator = Container::iterator;
  using const_iterator = Container::const_iterator;

  Container _data{};
  iterator _tail = _data.begin();
  Compare _comp;
};