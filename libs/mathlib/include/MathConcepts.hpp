//
// MathConcepts.hpp
// mathlib
//
// Created by Usama Alshughry 19.07.2024.
// Copyright © 2024 Usama Alshughry. All rights reserved.
//

#ifndef MATHCONCEPTS_HPP_
#define MATHCONCEPTS_HPP_

#include <concepts>

namespace math
{

template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept FloatingPoint = std::is_floating_point_v<T>;

template <typename T>
concept Integral = std::is_integral_v<T>;

} // namespace math

#endif // MATHCONCEPTS_HPP_
