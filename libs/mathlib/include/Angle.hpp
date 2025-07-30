//
// AngleT.hpp
// mathlib
//
// Created by Usama Alshughry 21.07.2024.
// Copyright © 2024 Usama Alshughry. All rights reserved.
//

#ifndef ANGLE_HPP_
#define ANGLE_HPP_

#include "MyTypes.hpp"
#include "MathConcepts.hpp"
#include <compare>
#include <cmath>

namespace math
{

class Angle;

namespace literals
{

  constexpr math::Angle operator"" _deg(u64 const);
  constexpr math::Angle operator"" _rad(u64 const);
  constexpr math::Angle operator"" _deg(f128 const);
  constexpr math::Angle operator"" _rad(f128 const);

} // namespace literals

class Angle
{
public:
  [[nodiscard]] constexpr Angle(Angle const&) = default;
  [[nodiscard]] constexpr Angle(Angle&&) = default;
  constexpr Angle& operator=(Angle const&) = default;
  constexpr Angle& operator=(Angle&&) = default;
  // getters
  [[nodiscard]] inline constexpr
  f64 asDegrees() const
  {
    return m_degrees;
  }

  [[nodiscard]] inline constexpr
  f64 asRadians() const
  {
    return m_degrees * ToRadians;
  }

  // modifying operators
  inline constexpr
  Angle& operator+=(Angle const& a)
  {
    m_degrees += a.m_degrees;
    return *this;
  }

  inline constexpr
  Angle& operator-=(Angle const& a)
  {
    m_degrees -= a.m_degrees;
    return *this;
  }

  inline constexpr
  Angle& operator*=(f64 const scalar)
  {
    m_degrees *= scalar;
    return *this;
  }

  inline constexpr
  Angle& operator/=(f64 const divisor)
  {
    m_degrees /= divisor;
    return *this;
  }

  // non-modifying operators
  [[nodiscard]] inline constexpr
  Angle operator+(Angle const& a) const
  {
    return Angle(m_degrees + a.m_degrees);
  }

  [[nodiscard]] inline constexpr
  Angle operator-(Angle const& a) const
  {
    return Angle(m_degrees - a.m_degrees);
  }

  [[nodiscard]] inline constexpr
  Angle operator*(f64 const scalar) const
  {
    return Angle(m_degrees * scalar);
  }

  [[nodiscard]] inline constexpr
  Angle operator/(f64 const divisor) const
  {
    return Angle(m_degrees / divisor);
  }

  [[nodiscard]] inline constexpr
  f64 operator/(Angle const& a) const
  {
    return m_degrees / a.m_degrees;
  }

  [[nodiscard]] inline constexpr
  Angle operator%(Angle const& a) const
  {
    Angle result(m_degrees);
    result.positiveRemainder(a.m_degrees);
    return result;
  }

  inline constexpr
  Angle operator-() const
  {
    return Angle(-m_degrees);
  }

  inline constexpr
  Angle& wrapSigned()
  {
    m_degrees += 180;
    wrapUnsigned();
    m_degrees -= 180;
    return *this;
  }

  inline constexpr
  Angle& wrapUnsigned()
  {
    positiveRemainder(360.f);
    return *this;
  }

  [[nodiscard]] inline constexpr
  f64 sin() const
  {
    return std::sin(asRadians());
  }

  [[nodiscard]] inline constexpr
  f64 cos() const
  {
    return std::cos(asRadians());
  }

  [[nodiscard]] inline constexpr
  f64 tan() const
  {
    return std::tan(asRadians());
  }

  constexpr auto operator<=>(Angle const& other) const = default;

private:
  constexpr Angle(f64 const degrees) : m_degrees{degrees} { }

  void positiveRemainder(f64 const b)
  {
    f64 val = m_degrees - static_cast<f64>(static_cast<i64>(m_degrees / b)) * b;
    m_degrees = val >= 0 ? val : val + b;
  }

  f64 m_degrees;
  static constexpr f64 ToRadians = 0.017453292519943;
  static constexpr f64 ToDegrees = 57.295779513082321;

  friend inline constexpr Angle degrees(f64 const);
  friend inline constexpr Angle radians(f64 const);

  friend constexpr Angle literals::operator"" _deg(u64);
  friend constexpr Angle literals::operator"" _rad(u64);
  friend constexpr Angle literals::operator"" _deg(f128);
  friend constexpr Angle literals::operator"" _rad(f128);
};

[[nodiscard]] inline constexpr
Angle degrees(f64 const d)
{
  return Angle(d);
}

[[nodiscard]] inline constexpr
Angle radians(f64 const r)
{
  return Angle(r * Angle::ToDegrees);
}

namespace literals
{

  [[nodiscard]] constexpr
  math::Angle operator"" _deg(u64 const d)
  {
    return math::Angle(d);
  }

  [[nodiscard]] constexpr
  math::Angle operator"" _rad(u64 const r)
  {
    return math::Angle(r * math::Angle::ToDegrees);
  }

  [[nodiscard]] constexpr
  math::Angle operator"" _deg(f128 const d)
  {
    return math::Angle(d);
  }

  [[nodiscard]] constexpr
  math::Angle operator"" _rad(f128 const r)
  {
    return math::Angle(r * math::Angle::ToDegrees);
  }

} // namespace literals

} // namespace math

using namespace math::literals;

#endif // ANGLE_HPP_
