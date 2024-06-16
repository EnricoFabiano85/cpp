#pragma once

#include "Expression.h"

template<typename T>
concept IsExpression = std::is_base_of_v<Expression, T>;