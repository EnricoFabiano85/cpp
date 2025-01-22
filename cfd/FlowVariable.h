#pragma once

#include "StorageManager.h"

class FlowVariable
{
public:
  FlowVariable(auto &&s) requires (std::convertible_to<decltype(s), std::string>) 
    : _span((StorageManager::getStorageManager()->getVariable(std::forward<std::string>(s)))) 
  {}

  inline decltype(auto) operator[](auto &&index)
  { return _span[index._i, index._j]; }

private:
  std::mdspan<double, std::dextents<size_t, 2>> _span;
};