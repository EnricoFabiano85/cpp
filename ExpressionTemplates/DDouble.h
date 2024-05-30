#pragma once

#include "Expression.h"
#include <concepts>
#include <type_traits>

template<typename T>
concept ExpressionIsBase = std::is_base_of_v<Expression<T>, T>;

struct DDouble : public Expression<DDouble>
{
	double _value;

	constexpr DDouble(double const value) : _value(value)
	{}

	template<ExpressionIsBase T>
	DDouble(T const &rhs) : _value(rhs.apply())
	{}

	auto &primitive() const
	{ return _value; }

	auto &apply_impl() const
	{ return primitive(); }
};