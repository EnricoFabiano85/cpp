#pragma once

#include "Expression.h"

#include <type_traits>

template<typename T>
concept IsExpression = std::is_base_of_v<Expression, T>;