#pragma once

#include <algorithm>
#include <execution>
#include <ranges>

#include "Index.h"
#include "Mesh.h"

#define Loop(INDEX, DIR, POLICY)    \
{                           \
  auto const idxs = std::views::iota(0, MeshData::nCells); \
  std::for_each(POLICY, std::begin(idxs), std::end(idxs), [&](int const cell) \
  {                                       \
    auto const I = cell/MeshData::cols;                \
    auto const J = cell%MeshData::cols;                \
    auto INDEX = Index<DIR>{._i=I, ._j=J}; \

#define LoopEnd()   \
  });               \
  }                 \


#define LoopX(INDEX, POLICY) Loop(INDEX, X, POLICY)
#define LoopY(INDEX, POLICY) Loop(INDEX, Y, POLICY)
#define LoopZ(INDEX, POLICY) Loop(INDEX, Z, POLICY)