//
// MathHelper.hpp
// mathlib
//
// Created by Usama Alshughry 21.07.2024.
// Copyright © 2024 Usama Alshughry. All rights reserved.
//

#ifndef MATHHELPER_HPP_
#define MATHHELPER_HPP_

#include "MyTypes.hpp"
#include "MathConcepts.hpp"
#include <cmath>
#include <algorithm>

namespace math
{

using std::min;
using std::max;
using std::abs;
using std::clamp;

template <FloatingPoint T>
static constexpr T Epsilon = 1.e-6;

template <FloatingPoint T>
inline constexpr bool isZero(T a)
{
  return abs(a) < Epsilon<T>;
}

} // namespace math

#endif // MATHHELPER_HPP_
