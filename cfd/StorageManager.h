#pragma once

#include "mdspan.hpp"
#include "Mesh.h"

#include <string>
#include <vector>
#include <unordered_map>

#include <iostream>

class StorageManager
{
  StorageManager() = default;
  ~StorageManager() = default;

  inline static StorageManager* _instance{nullptr};

  std::unordered_map<std::string, std::vector<double>> _theStorage{};

public:
  StorageManager(StorageManager const &) = delete;
  StorageManager &operator=(StorageManager const &) = delete;

  static StorageManager* getStorageManager()
  {
    if (!_instance)
      _instance = new StorageManager();

    return _instance;
  }

  auto addVariable(auto &&s)
  requires (std::convertible_to<decltype(s), std::string>)
  { _theStorage[std::forward<std::string>(s)].resize(MeshData::nCells); }

  auto getVariable(auto &&s)
  requires (std::convertible_to<decltype(s), std::string>)
  { return std::experimental::mdspan(_theStorage[std::forward<std::string>(s)].data(), MeshData::rows, MeshData::cols); }
};