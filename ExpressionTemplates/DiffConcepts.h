#pragma once

#include "DiffMode.h"
#include "Expression.h"
#include <type_traits>

#include <type_traits>

template<typename T>
concept IsExpression = std::is_base_of_v<Expression, T>;

template<typename T>
concept PrimalMode = (T::DMode == DiffMode::PRIMAL);

template<typename T>
concept TangentMode = (T::DMode == DiffMode::TANGENT);

template<typename T>
concept ForwardExpression = IsExpression<T> && (PrimalMode<T> || TangentMode<T>);