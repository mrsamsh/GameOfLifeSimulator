//
// Vector.hpp
// mathlib
//
// Created by Usama Alshughry 19.07.2024.
// Copyright © 2024 Usama Alshughry. All rights reserved.
//

#ifndef VECTOR_HPP_
#define VECTOR_HPP_

#include "MyTypes.hpp"
#include "MathConcepts.hpp"
#include "MathHelper.hpp"

namespace math
{

template <Arithmetic T, usz N>
class VectorT
{
public:
  using value_t = T;
  constexpr VectorT() : elements{0} { }
  template <Arithmetic ... Ts>
  requires (sizeof...(Ts) == N)
  constexpr VectorT(Ts ... values) : elements{static_cast<T>(values)...} { }
  template <Arithmetic U>
  constexpr explicit VectorT(VectorT<U, N> const& v) : elements{v.elements} { }
  template <usz M>
  requires (M != N)
  constexpr explicit VectorT(VectorT<T, M> const& v) : elements{0}
  {
    memcpy(elements, v.data(), sizeof(T) * (M > N ? N : M));
  }

  template <usz M, usz P>
  requires (M + P == N)
  constexpr explicit VectorT(VectorT<T, M> const& v1, VectorT<T, P> const& v2)
  : elements{0}
  {
    memcpy(elements, v1.data(), sizeof(T) * M);
    memcpy(elements + M, v2.data(), sizeof(T) * P);
  }

  inline constexpr T& operator[](usz i) { return elements[i]; }
  inline constexpr T const& operator[](usz i) const { return elements[i]; }
  inline constexpr T* data() { return elements; }
  inline constexpr T const* data() const { return elements; }

  inline static constexpr usz size() { return N; }

  // modifying operators
  inline constexpr VectorT& operator+=(VectorT const& rhs)
  {
    for (usz i = 0; i < N; ++i)
      elements[i] += rhs.elements[i];
    return *this;
  }

  inline constexpr VectorT& operator-=(VectorT const& rhs)
  {
    for (usz i = 0; i < N; ++i)
      elements[i] -= rhs.elements[i];
    return *this;
  }

  inline constexpr VectorT& operator*=(VectorT const& rhs)
  {
    for (usz i = 0; i < N; ++i)
      elements[i] *= rhs.elements[i];
    return *this;
  }

  inline constexpr VectorT& operator/=(VectorT const& rhs)
  {
    for (usz i = 0; i < N; ++i)
      elements[i] /= rhs.elements[i];
    return *this;
  }

  inline constexpr VectorT& operator*=(T const scalar)
  {
    for (usz i = 0; i < N; ++i)
      elements[i] *= scalar;
    return *this;
  }

  inline constexpr VectorT& operator/=(T const divisor)
  {
    for (usz i = 0; i < N; ++i)
      elements[i] /= divisor;
    return *this;
  }

private:
  value_t elements[N];
};

template <Arithmetic T>
class VectorT<T, 2>
{
public:
  using value_t = T;
  constexpr VectorT() : x(0), y(0) { }
  constexpr VectorT(T a, T b) : x{a}, y{b} { }
  template <Arithmetic U>
  constexpr VectorT(U a, U b) : x(a), y(b) { }
  template <Arithmetic U>
  constexpr explicit VectorT(VectorT<U, 2> const& v)
  : x{static_cast<T>(v.x)}
  , y{static_cast<T>(v.y)} { }
  template <usz N>
  requires (N > 2)
  constexpr explicit VectorT(VectorT<T, N> const& v) : x{v[0]}, y{v[1]} { }

  // modifying operators
  inline constexpr VectorT& operator+=(VectorT const& rhs)
  {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }

  inline constexpr VectorT& operator-=(VectorT const& rhs)
  {
    x -= rhs.x;
    y -= rhs.y;
    return *this;
  }

  inline constexpr VectorT& operator*=(VectorT const& rhs)
  {
    x *= rhs.x;
    y *= rhs.y;
    return *this;
  }

  inline constexpr VectorT& operator/=(VectorT const& rhs)
  {
    x /= rhs.x;
    y /= rhs.y;
    return *this;
  }

  inline constexpr VectorT& operator*=(T const scalar)
  {
    x *= scalar;
    y *= scalar;
    return *this;
  }

  inline constexpr VectorT& operator/=(T const divisor)
  {
    x /= divisor;
    y /= divisor;
    return *this;
  }

  inline constexpr T& operator[](usz i)
  {
    switch (i)
    {
    case 0: return x;
    case 1: return y;
    default: return x;
    }
  }

  inline constexpr T const & operator[](usz i) const
  {
    switch (i)
    {
    case 0: return x;
    case 1: return y;
    default: return x;
    }
  }

  inline constexpr T* data() { return &x; }

  inline constexpr T const* data() const { return &x; }

  inline static constexpr usz size() { return 2; }

  value_t x, y;
};

template <Arithmetic T>
class VectorT<T, 3>
{
public:
  using value_t = T;
  constexpr VectorT() : x(0), y(0), z(0) { }
  constexpr VectorT(T a, T b, T c) : x{a}, y{b}, z{c} { }
  constexpr VectorT(VectorT<T, 2> const& v, T c) : x{v.x}, y{v.y}, z{c} { }
  constexpr VectorT(T a, VectorT<T, 2> const& v) : x{a}, y{v.x}, z{v.y} { }
  template <usz N>
  requires (N > 3)
  constexpr explicit VectorT(VectorT<T, N> const& v)
  : x{v[0]}, y{v[1]}, z{v[2]} { }

  template <Arithmetic U>
  constexpr VectorT(U a, U b, U c) : x(a), y(b), z(c) { }
  template <Arithmetic U>
  constexpr explicit VectorT(VectorT<U, 3> const& v)
  : x{static_cast<T>(v.x)}
  , y{static_cast<T>(v.y)}
  , z{static_cast<T>(v.z)} { }

  // modifying operators
  inline constexpr VectorT& operator+=(VectorT const& rhs)
  {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
  }

  inline constexpr VectorT& operator-=(VectorT const& rhs)
  {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
  }

  inline constexpr VectorT& operator*=(VectorT const& rhs)
  {
    x *= rhs.x;
    y *= rhs.y;
    z *= rhs.z;
    return *this;
  }

  inline constexpr VectorT& operator/=(VectorT const& rhs)
  {
    x /= rhs.x;
    y /= rhs.y;
    z /= rhs.z;
    return *this;
  }

  inline constexpr VectorT& operator*=(T const scalar)
  {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
  }

  inline constexpr VectorT& operator/=(T const divisor)
  {
    x /= divisor;
    y /= divisor;
    z /= divisor;
    return *this;
  }

  inline constexpr T& operator[](usz i)
  {
    switch (i)
    {
    case 0: return x;
    case 1: return y;
    case 2: return z;
    default: return x;
    }
  }

  inline constexpr T const & operator[](usz i) const
  {
    switch (i)
    {
    case 0: return x;
    case 1: return y;
    case 2: return z;
    default: return x;
    }
  }

  inline constexpr T* data() { return &x; }

  inline constexpr T const* data() const { return &x; }

  inline static constexpr usz size() { return 3; }

  value_t x, y, z;
};

template <Arithmetic T>
class VectorT<T, 4>
{
public:
  using value_t = T;
  constexpr VectorT() : x(0), y(0), z(0), w(0) { }
  constexpr VectorT(T a, T b, T c, T d) : x{a}, y{b}, z{c}, w{d} { }
  constexpr VectorT(VectorT<T, 3> const& v, T d)
  : x{v.x}, y{v.y}, z{v.z}, w{d} { }
  constexpr VectorT(T a, VectorT<T, 3> const& v)
  : x{a}, y{v.x}, z{v.y}, w{v.z} { }
  constexpr VectorT(VectorT<T, 2> const& v, T c, T d)
  : x{v.x}, y{v.y}, z{c}, w{d} { }
  constexpr VectorT(T a, VectorT<T, 2> const& v, T d)
  : x{a}, y{v.x}, z{v.y}, w{d} { }
  constexpr VectorT(T a, T b, VectorT<T, 2> const& v)
  : x{a}, y{b}, z{v.x}, w{v.y} { }

  template <usz N>
  requires (N > 4)
  constexpr explicit VectorT(VectorT<T, N> const& v)
  : x{v[0]}, y{v[1]}, z{v[2]}, w{v[3]} { }

  template <Arithmetic U>
  constexpr VectorT(U a, U b, U c, U d) : x(a), y(b), z(c), w(d) { }
  template <Arithmetic U>
  constexpr explicit VectorT(VectorT<U, 4> const& v)
  : x{static_cast<T>(v.x)}
  , y{static_cast<T>(v.y)}
  , z{static_cast<T>(v.z)}
  , w{static_cast<T>(v.w)} { }


  // modifying operators
  inline constexpr VectorT& operator+=(VectorT const& rhs)
  {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    w += rhs.w;
    return *this;
  }

  inline constexpr VectorT& operator-=(VectorT const& rhs)
  {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    w -= rhs.w;
    return *this;
  }

  inline constexpr VectorT& operator*=(VectorT const& rhs)
  {
    x *= rhs.x;
    y *= rhs.y;
    z *= rhs.z;
    w *= rhs.w;
    return *this;
  }

  inline constexpr VectorT& operator/=(VectorT const& rhs)
  {
    x /= rhs.x;
    y /= rhs.y;
    z /= rhs.z;
    w /= rhs.w;
    return *this;
  }

  inline constexpr VectorT& operator*=(T const scalar)
  {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;
    return *this;
  }

  inline constexpr VectorT& operator/=(T const divisor)
  {
    x /= divisor;
    y /= divisor;
    z /= divisor;
    w /= divisor;
    return *this;
  }

  inline constexpr T& operator[](usz i)
  {
    switch (i)
    {
    case 0: return x;
    case 1: return y;
    case 2: return z;
    case 3: return w;
    default: return x;
    }
  }

  inline constexpr T const & operator[](usz i) const
  {
    switch (i)
    {
    case 0: return x;
    case 1: return y;
    case 2: return z;
    case 3: return w;
    default: return x;
    }
  }

  inline constexpr T* data() { return &x; }

  inline constexpr T const* data() const { return &x; }

  inline static constexpr usz size() { return 4; }

  value_t x, y, z, w;
};

template <Arithmetic T, usz N>
inline constexpr
T sumComponents(VectorT<T, N> const& v)
{
  T result = 0;
  for (usz i = 0; i < N; ++i)
  {
    result += v[i];
  }
  return result;
}

template <Arithmetic T>
inline constexpr
T sumComponents(VectorT<T, 2> const& v)
{
  return v.x + v.y;
}

template <Arithmetic T>
inline constexpr
T sumComponents(VectorT<T, 3> const& v)
{
  return v.x + v.y + v.z;
}

template <Arithmetic T>
inline constexpr
T sumComponents(VectorT<T, 4> const& v)
{
  return v.x + v.y + v.z + v.w;
}

// non-modifying operators
template <Arithmetic T, usz N>
[[nodiscard]] inline constexpr
VectorT<T, N> operator+(VectorT<T, N> l, VectorT<T, N> const& r)
{
  return l += r;
}

template <Arithmetic T, usz N>
[[nodiscard]] inline constexpr
VectorT<T, N> operator-(VectorT<T, N> l, VectorT<T, N> const& r)
{
  return l -= r;
}

template <Arithmetic T, usz N>
[[nodiscard]] inline constexpr
VectorT<T, N> operator*(VectorT<T, N> l, VectorT<T, N> const& r)
{
  return l *= r;
}

template <Arithmetic T, usz N>
[[nodiscard]] inline constexpr
VectorT<T, N> operator/(VectorT<T, N> l, VectorT<T, N> const& r)
{
  return l /= r;
}

template <Arithmetic T, usz N>
[[nodiscard]] inline constexpr
VectorT<T, N> operator*(VectorT<T, N> l, typename VectorT<T, N>::value_t const scalar)
{
  return l *= scalar;
}

template <Arithmetic T, usz N>
[[nodiscard]] inline constexpr
VectorT<T, N> operator*(typename VectorT<T, N>::value_t const scalar, VectorT<T, N> l)
{
  return l *= scalar;
}

template <Arithmetic T, usz N>
[[nodiscard]] inline constexpr
VectorT<T, N> operator/(VectorT<T, N> l, typename VectorT<T, N>::value_t const divisor)
{
  return l /= divisor;
}

template <Arithmetic T, usz N>
[[nodiscard]] inline constexpr
VectorT<T, N> operator/(typename VectorT<T, N>::value_t const divisor, VectorT<T, N> r)
{
  for (usz i = 0; i < N; ++i)
    r[i] = divisor / r[i];
  return r;
}

template <Arithmetic T, usz N>
inline constexpr
T dotProduct(VectorT<T, N> const& l, VectorT<T, N> const& r)
{
  return sumComponents(l * r);
}

template <Arithmetic T, usz N>
inline constexpr
T length2(VectorT<T, N> const& v)
{
  return dotProduct(v, v);
}

template <Arithmetic T, usz N>
inline constexpr
T length(VectorT<T, N> const& v)
{
  return std::sqrt(dotProduct(v, v));
}

template <Arithmetic T>
inline constexpr
VectorT<T, 3> crossProduct(VectorT<T, 3> const& l, VectorT<T, 3> const& r)
{
  return {
    l.y * r.z - l.z * r.y,
    l.z * r.x - l.x * r.z,
    l.x * r.y - l.y * r.x
  };
}

template <Arithmetic T, usz N>
inline constexpr
VectorT<T, N> operator-(VectorT<T, N> const& v)
{
  return v * static_cast<T>(-1);
}

template <Integral T, usz N>
inline constexpr
bool isZero(VectorT<T, N> const& v)
{
  for (usz i = 0; i < N; ++i)
    if (v[i] != 0)
      return false;
  return true;
}

template <Integral T>
inline constexpr
bool isZero(VectorT<T, 2> const& v)
{
  return v.x == 0 && v.y == 0;
}

template <Integral T>
inline constexpr
bool isZero(VectorT<T, 3> const& v)
{
  return v.x == 0 && v.y == 0 && v.z == 0;
}

template <Integral T>
inline constexpr
bool isZero(VectorT<T, 4> const& v)
{
  return v.x == 0 && v.y == 0 && v.z == 0 && v.w == 0;
}

template <FloatingPoint T, usz N>
inline constexpr
bool isZero(VectorT<T, N> const& v)
{
  for (usz i = 0; i < N; ++i)
    if (!isZero(v[i]))
      return false;
  return true;
}

template <FloatingPoint T>
inline constexpr
bool isZero(VectorT<T, 2> const& v)
{
  return isZero(v.x) && isZero(v.y);
}

template <FloatingPoint T>
inline constexpr
bool isZero(VectorT<T, 3> const& v)
{
  return isZero(v.x) && isZero(v.y) && isZero(v.z);
}

template <FloatingPoint T>
inline constexpr
bool isZero(VectorT<T, 4> const& v)
{
  return isZero(v.x) && isZero(v.y) && isZero(v.z) && isZero(v.w);
}

template <Arithmetic T, usz N>
inline constexpr
bool operator==(VectorT<T, N> const& l, VectorT<T, N> const& r)
{
  return isZero(l - r);
}

template <Arithmetic T, usz N>
inline constexpr
bool operator!=(VectorT<T, N> const& l, VectorT<T, N> const& r)
{
  return !isZero(l - r);
}

template <Arithmetic T, usz N>
inline constexpr
VectorT<T, N> normalize(VectorT<T, N> const& v)
{
  if (isZero(v))
    return VectorT<T, N>();
  else
    return v / length(v);
}

template <Arithmetic T, usz N>
[[nodiscard]] inline constexpr
VectorT<T, N> reflect(VectorT<T, N> const& v, VectorT<T, N> const& n)
{
  VectorT<T, N> nn = normalize(n);
  return v - 2 * dotProduct(v, nn) * nn;
}

} // namespace math

#endif // VECTOR_HPP_
