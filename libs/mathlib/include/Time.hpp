//
// Time.hpp
// mathlib
//
// Created by Usama Alshughry 21.07.2024.
// Copyright © 2024 Usama Alshughry. All rights reserved.
//

#ifndef TIME_HPP_
#define TIME_HPP_

#include "MyTypes.hpp"

#include <compare>

namespace math
{

class Time;

namespace literals
{

  constexpr math::Time operator"" _sec(u64 const);
  constexpr math::Time operator"" _sec(f128 const);
  constexpr math::Time operator"" _mlsec(u64 const);
  constexpr math::Time operator"" _mlsec(f128 const);
  constexpr math::Time operator"" _mcsec(u64 const);
  constexpr math::Time operator"" _mcsec(f128 const);
  constexpr math::Time operator"" _nsec(u64 const);
  constexpr math::Time operator"" _nsec(f128 const);

} // namespace literals

class Time
{
public:
  [[nodiscard]] inline constexpr
  f64 asSeconds() const
  {
    return m_nanoseconds * 1.e-9;
  }

  [[nodiscard]] inline constexpr
  u64 asMilliseconds() const
  {
    return m_nanoseconds * 1.e-6;
  }

  [[nodiscard]] inline constexpr
  u64 asMicroseconds() const
  {
    return m_nanoseconds * 1.e-3;
  }

  [[nodiscard]] inline constexpr
  u64 asNanoseconds() const
  {
    return m_nanoseconds;
  }

  inline constexpr
  Time& operator+=(Time const& r)
  {
    m_nanoseconds += r.m_nanoseconds;
    return *this;
  }

  inline constexpr
  Time& operator-=(Time const& r)
  {
    m_nanoseconds -= r.m_nanoseconds;
    return *this;
  }

  inline constexpr
  Time& operator*=(f64 const scalar)
  {
    m_nanoseconds *= scalar;
    return *this;
  }

  inline constexpr
  Time& operator/=(f64 const divisor)
  {
    m_nanoseconds /= divisor;
    return *this;
  }

  [[nodiscard]] inline constexpr
  Time operator+(Time const& r) const
  {
    return Time(m_nanoseconds + r.m_nanoseconds);
  }

  [[nodiscard]] inline constexpr
  Time operator-(Time const& r) const
  {
    return Time(m_nanoseconds - r.m_nanoseconds);
  }

  [[nodiscard]] inline constexpr
  Time operator*(f64 const scalar) const
  {
    return Time(m_nanoseconds * scalar);
  }

  [[nodiscard]] inline constexpr
  Time operator/(f64 const divisor) const
  {
    return Time(m_nanoseconds / divisor);
  }

  auto operator<=>(Time const& r) const
  {
    return m_nanoseconds <=> r.m_nanoseconds;
  }

private:
  constexpr Time(u64 ns) : m_nanoseconds{ns} { }
  u64 m_nanoseconds;

private:
  friend inline constexpr Time seconds(f64 const s);
  friend inline constexpr Time milliseconds(u64 const ms);
  friend inline constexpr Time microseconds(u64 const mcs);
  friend inline constexpr Time nanoseconds(u64 const ns);

  friend constexpr Time literals::operator"" _sec(u64 const);
  friend constexpr Time literals::operator"" _sec(f128 const);
  friend constexpr Time literals::operator"" _mlsec(u64 const);
  friend constexpr Time literals::operator"" _mlsec(f128 const);
  friend constexpr Time literals::operator"" _mcsec(u64 const);
  friend constexpr Time literals::operator"" _mcsec(f128 const);
  friend constexpr Time literals::operator"" _nsec(u64 const);
  friend constexpr Time literals::operator"" _nsec(f128 const);
};

inline constexpr Time seconds(f64 const s)
{
  return Time(s * 1.e+9);
}

inline constexpr Time milliseconds(u64 const ms)
{
  return Time(ms * 1.e+6);
}

inline constexpr Time microseconds(u64 const mcs)
{
  return Time(mcs * 1.e+3);
}

inline constexpr Time nanoseconds(u64 const ns)
{
  return Time(ns);
}

namespace literals
{

  constexpr math::Time operator"" _sec(u64 const s)
  {
    return math::seconds(s);
  }
  constexpr math::Time operator"" _sec(f128 const s)
  {
    return math::seconds(s);
  }
  constexpr math::Time operator"" _mlsec(u64 const ms)
  {
    return math::milliseconds(ms);
  }
  constexpr math::Time operator"" _mlsec(f128 const ms)
  {
    return math::milliseconds(ms);
  }
  constexpr math::Time operator"" _mcsec(u64 const mcs)
  {
    return math::microseconds(mcs);
  }
  constexpr math::Time operator"" _mcsec(f128 const mcs)
  {
    return math::microseconds(mcs);
  }
  constexpr math::Time operator"" _nsec(u64 const ns)
  {
    return Time(ns);
  }
  constexpr math::Time operator"" _nsec(f128 const ns)
  {
    return Time(ns);
  }

} // namespace literals

} // namespace math

using namespace math::literals;

#endif // TIME_HPP_
