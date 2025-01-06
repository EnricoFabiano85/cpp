/*
  Rough implementation of hash table with functionalities similar
  to unordered_multimap. Will also be tested on
  https://leetcode.com/problems/design-hashmap/description/

  Notes: std::unordered_map is equivalent to a
  std::forward_list<Map_Node> + std::vector<std::forward_list<Map_Node>::iterator>
  (although everything is done without containers, i.e. using raw poiters). 
  Essentially this is an exercise at implementing an unordered map using stl containers.
  Both here and in std::unordered_map the insert function inserts a node at the beginning
  of the linked list (using the before_begin node/iterator).
  In stl equal_range uses a for loop (and checks that keys are equal) 
  starting from the iterator in the bucket, while here I store two iterators (std::pair), 
  one for the beginning and one for past the end of the bucket (could be end of the 
  linked list). An alternative implementation could have been to follow stl (one iterator)
  per bucket and use ranges to find the end of the bucket by filtering on the keys. However,
  this approach seems to require to capture the key by value when filtering (see below).
  
  Don't like: I'm sort of exposing the details of the implementation (see how Node is used) and also 
  I am not using move semantics anywhere.
*/

#include <algorithm>
#include <cstddef>
#include <forward_list>
#include <iostream>
#include <iterator>
#include <utility>
#include <vector>
#include <functional>

template<typename Key, typename Value, typename Hash = std::hash<Key>>
class UnorderedMMap
{
  struct Node
  {
    Key const _key;
    Value const _value;
  };

  public: 

  using Iterator = typename std::forward_list<Node>::iterator;
  using IteratorPair = std::pair<Iterator, Iterator>;
  using value_type = std::pair<Key, Value>;

  UnorderedMMap() : _list()
                  , _bkt(_bucketCount, std::make_pair(_list.end(), _list.end()))
  {}

  UnorderedMMap(std::size_t const bucketCount) : _bucketCount(bucketCount)
                                                , _list()
                                                , _bkt(_bucketCount, std::make_pair(_list.end(), _list.end()))
  {}

  void insert(value_type const &v)
  { 
    auto const &key = v.first;
    auto const &value = v.second;

    auto &pair = _bkt[getHash(key)];

    if (pair.first != _list.end())
    {
      auto previousNode = findBeforeNode(pair.first, pair.second, pair.first, key);
      _list.insert_after(previousNode, {key, value});
    }
    else
    {
      if (!_list.empty()) ++_bkt[getHash(_list.front()._key)].first;
      
      pair.first = _list.before_begin();
      pair.second = _list.insert_after(pair.first, {key, value});
      ++pair.second;
    }

    ++_size;

    if (loadFactor() > maxLoadFactor()) rehash(2*_bucketCount);
  }

  auto equal_range(Key const& key) const
  {
    auto const [first, second] = _bkt[getHash(key)];

    auto const it1 = std::find_if(first, second, [&key](auto const &n){ return (n._key == key); });
    auto it2 = it1;

    while(it2 != second && it2->_key == key) ++it2;

    return std::make_pair(it1, it2);

    //This could also have been done with ranges. For that, I would need to capture
    // key by value in the lambda otherwise the range is invalid when returning:
    // return (std::ranges::subrange(it1, it2) | std::views::filter([&key](auto const &n){ 
    //   return (n._key == key);
    //   }));
  }

  auto inline getHash(Key const &key) const
  { return Hash{}(key) % _bucketCount; }

  auto erase(Key const &key)
  {
    // Erase by key scales as the number of nodes with that (hashed) key, however there is the extra cost
    // of finding the fwd list iterator that points to the node before the beginning of the
    // bkt with that key (bcoz with fwd list you erase after). Some optimizations are possible 
    // (list empty, bkt empty, key at front of list), but ultimately the cost here is that of 
    // searching a list + removing elements (see /usr/include/c++/14/bits/hashtable.h erase and _M_find_before_node)
    // Erase is O(1) if we have a pointer/iterator to the node that we want to delete (not supported here)

    if (_list.empty()) return size_t{0};

    auto const hashCode = getHash(key);
    auto &[firstNodeBkt, secondNodeBkt] = _bkt[hashCode];

    if (firstNodeBkt == _list.end()) return size_t{0};
    
    auto beforeNode = findBeforeNode(_list.begin(), 
                                     secondNodeBkt, 
                                     _list.before_begin(), 
                                     key);

    auto nextNode = std::next(beforeNode);
    std::size_t nErasedElements = 0;
    while (nextNode != secondNodeBkt) 
    {
      if (nextNode->_key == key)
      {
        auto afterErasedNode = _list.erase_after(beforeNode);

        if (nextNode == firstNodeBkt) firstNodeBkt = afterErasedNode;
        if (firstNodeBkt != _list.end() && getHash(firstNodeBkt->_key) != hashCode )
        {
          firstNodeBkt = _list.end();
          secondNodeBkt = _list.end();
        }

        nextNode = afterErasedNode;
        ++nErasedElements;
        --_size;
      }
      else
        ++nextNode;
    }

    return nErasedElements;
  }

  void rehash(std::size_t n)
  {
    auto newBktCount = std::max({n, _bucketCount, std::size_t(_size/_maxLoadFactor)+1});

    std::vector<IteratorPair> newBkt(newBktCount);
    _bucketCount = newBktCount;

    auto bucketBegin = _list.before_begin();
    auto listIterator = _list.begin();
    while(listIterator != _list.end())
    {
      auto previousNode = bucketBegin;
      auto const key = listIterator->_key;

      for (; listIterator != _list.end(); ++listIterator)
      {
        if (listIterator->_key != key) break;
        ++previousNode;
      }

      auto &bucket = newBkt[getHash(key)];
      bucket.first = bucketBegin;
      bucket.second = (previousNode == _list.end()) ? _list.end() : ++previousNode;
      bucketBegin = listIterator;
    }

    _bkt.swap(newBkt);
  }

  std::size_t bucketSize(std::size_t const n) const noexcept
  { 
    if (n > _bucketCount) return 0;
    return std::distance(_bkt[n].first, _bkt[n].second); 
  }

  std::size_t inline bucketCount() const noexcept
  { return _bucketCount; }

  float inline loadFactor() const
  { return float(_size)/float(bucketCount()); }

  float inline maxLoadFactor() const noexcept
  { return _maxLoadFactor; }

  void maxLoadFactor(float const loadFactor) const noexcept
  { _maxLoadFactor = loadFactor; }


  private:

  auto findBeforeNode(Iterator first, Iterator last, Iterator startFrom, Key const &key) const
  {
    Iterator previousNode = startFrom;
    Iterator previousNodeBeforeUpdate;
    for (; first != last; ++first)
    {
      if (first->_key == key) return previousNode;

      previousNodeBeforeUpdate = previousNode;
      ++previousNode;
    }

    return previousNodeBeforeUpdate;
  }

  std::size_t _bucketCount{50};
  std::size_t _size{0};
  float mutable _maxLoadFactor{0.75};
  std::forward_list<Node> _list;
  std::vector<IteratorPair> _bkt;
};


int main()
{
  auto mmap = UnorderedMMap<int, int>(2);

  mmap.insert(std::make_pair(2, 42));
  mmap.insert(std::make_pair(1, 14424));
  mmap.insert(std::make_pair(2, 1042));
  mmap.insert(std::make_pair(2, 10421));
  mmap.insert(std::make_pair(4, 4424));
  mmap.insert(std::make_pair(4, 442));
  mmap.insert(std::make_pair(50, 50442));
  mmap.insert(std::make_pair(51, 5151));

  auto const &pairKey1 = mmap.equal_range(1);
  std::for_each(pairKey1.first, pairKey1.second, [](auto const &n){std::cout <<"key 1: " << n._value  << std::endl;});
  std::cout << std::endl << std::endl;

  auto const &pairKey2 = mmap.equal_range(2);
  std::for_each(pairKey2.first, pairKey2.second, [](auto const &n){std::cout <<"key 2: " << n._value  << std::endl;});
  std::cout << std::endl << std::endl;

  auto const &pairKey4 = mmap.equal_range(4);
  std::for_each(pairKey4.first, pairKey4.second, [](auto const &n){std::cout <<"key 4: " << n._value  << std::endl;});
  std::cout << std::endl << std::endl;

  auto erasedElements = mmap.erase(1);
  std::cout << "Erased these many elements with key 1 " << erasedElements << std::endl;
  erasedElements = mmap.erase(4);
  std::cout << "Erased these many elements with key 4 " << erasedElements << std::endl;

  auto const &pairKey50 = mmap.equal_range(50);
  std::for_each(pairKey50.first, pairKey50.second, [](auto const &n){std::cout <<"key 50: " << n._value  << std::endl;});
  std::cout << std::endl << std::endl;

  return 0;
}