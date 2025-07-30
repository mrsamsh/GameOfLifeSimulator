//
// Quaternion.hpp
// mathlib
//
// Created by Usama Alshughry 23.07.2024.
// Copyright © 2024 Usama Alshughry. All rights reserved.
//

#ifndef QUATERNION_HPP_
#define QUATERNION_HPP_

#include "MathConcepts.hpp"
#include "Angle.hpp"
#include "Vector.hpp"
#include "Matrix.hpp"

namespace math
{

template <FloatingPoint T>
class QuaternionT
{
public:
  using value_t = T;
  constexpr QuaternionT() : elements{0} { }
  template <FloatingPoint ... Ts>
  requires (sizeof...(Ts) == 4)
  constexpr QuaternionT(Ts ... values)
  : elements{values...} { }
  template <FloatingPoint ... Ts>
  requires (sizeof...(Ts) == 3)
  constexpr QuaternionT(Angle th, Ts ... values)
  : QuaternionT(th, VectorT<T, 3>(values...)) { }
  constexpr QuaternionT(Angle th, VectorT<T, 3> const& axis)
  : w((th / 2).cos())
  , axis(math::normalize(axis) * (th / 2).sin()) { }

public:
  union
  {
    T elements[4];
    struct
    {
      T w;
      union
      {
        struct { T x, y, z; };
        VectorT<T, 3> axis;
      };
    };
  };
};

template <FloatingPoint T>
[[nodiscard]] inline constexpr
T dotProduct(QuaternionT<T> const& l, QuaternionT<T> const& r)
{
  return l.w * r.w + l.x * r.x + l.y * r.y + l.z * r.z;
}

template <FloatingPoint T>
[[nodiscard]] inline constexpr
QuaternionT<T> operator+(QuaternionT<T> const& l, QuaternionT<T> const& r)
{
  return {l.w + r.w, l.x + r.x, l.y + r.y, l.z + r.z};
}

template <FloatingPoint T>
[[nodiscard]] inline constexpr
QuaternionT<T> operator-(QuaternionT<T> const& l, QuaternionT<T> const& r)
{
  return {l.w - r.w, l.x - r.x, l.y - r.y, l.z - r.z};
}

template <FloatingPoint T>
[[nodiscard]] inline constexpr
QuaternionT<T> operator*(QuaternionT<T> const& l, T const scalar)
{
  return {l.w * scalar, l.x * scalar, l.y * scalar, l.z * scalar};
}

template <FloatingPoint T>
[[nodiscard]] inline constexpr
QuaternionT<T> operator/(QuaternionT<T> const& l, T const divisor)
{
  return {l.w / divisor, l.x / divisor, l.y / divisor, l.z / divisor};
}

template <FloatingPoint T>
[[nodiscard]] inline constexpr
QuaternionT<T> operator*(QuaternionT<T> const& q, QuaternionT<T> const& r)
{
  return
  {
    r.w * q.w - r.x * q.x - r.y * q.y - r.z * q.z,
    r.w * q.x + r.x * q.w - r.y * q.z + r.z * q.y, 
    r.w * q.y + r.x * q.z + r.y * q.w - r.z * q.x,
    r.w * q.z - r.x * q.y + r.y * q.x + r.z * q.w
  };
}

template <FloatingPoint T>
[[nodiscard]] inline constexpr
T length2(QuaternionT<T> const& q)
{
  return dotProduct(q, q);
}

template <FloatingPoint T>
[[nodiscard]] inline constexpr
T length(QuaternionT<T> const& q)
{
  return std::sqrt(dotProduct(q, q));
}

template <FloatingPoint T>
[[nodiscard]] inline constexpr
QuaternionT<T> normalize(QuaternionT<T> const& q)
{
  return q / length(q);
}

template <FloatingPoint T>
[[nodiscard]] inline constexpr
MatrixT<T, 4> getMatrix(QuaternionT<T> const& q)
{
  auto r = q;
  if (std::abs(length2(r) - 1.0) > math::Epsilon<T>)
    r = normalize(q);
  return
  {
    1 - 2 * (r.y * r.y + r.z * r.z),
    2 * (r.x * r.y + r.w * r.z),
    2 * (r.x * r.z - r.w * r.y),
    0,

    2 * (r.x * r.y - r.w * r.z),
    1 - 2 * (r.x * r.x + r.z * r.z),
    2 * (r.y * r.z + r.w * r.x),
    0,

    2 * (r.x * r.z + r.w * r.y),
    2 * (r.y * r.z - r.w * r.x),
    1 - 2 * (r.x * r.x + r.y * r.y),
    0,

    0, 0, 0, 1
  };
}

template <FloatingPoint T>
[[nodiscard]] inline constexpr
QuaternionT<T> slerp(QuaternionT<T> const& q0, QuaternionT<T> const& q1,
    // T t)
    typename QuaternionT<T>::value_t t)
{
  T dp = dotProduct(q0, q1);
  if (dp >= 1) return q0;
  T sin_omega = std::sqrt(1.0 - dp * dp);
  if (std::abs(sin_omega) < 0.001)
    return q0 * (1 - t) + q1 * t;
  T omega = acos(dp);
  T a = sin((1 - t) * omega) / sin_omega;
  T b = sin(t * omega) / sin_omega;
  return q0 * a + q1 * b;
}

} // namespace math

#endif // QUATERNION_HPP_
