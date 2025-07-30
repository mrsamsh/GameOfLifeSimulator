//
// Matrix.hpp
// mathlib
//
// Created by Usama Alshughry 19.07.2024.
// Copyright © 2024 Usama Alshughry. All rights reserved.
//

#ifndef MATRIX_HPP_
#define MATRIX_HPP_

#include "Vector.hpp"
#include "Angle.hpp"

#include <type_traits>

namespace math
{

template <Arithmetic T, usz N>
class MatrixT
{
public:
  using value_t = T;
  using vector_t = VectorT<T, N>;
  using mat = MatrixT<T, N>;
  constexpr MatrixT() : elements{0} { }
  template <Arithmetic ... Ts>
  requires (sizeof...(Ts) == N * N)
  constexpr MatrixT(Ts ... values) : elements{static_cast<T>(values)...} { }
  template <typename ... Vs>
  requires (sizeof...(Vs) == N && (std::same_as<vector_t, Vs> && ...))
  constexpr MatrixT(Vs&& ... vs) : vs{std::forward<Vs>(vs)...} { }
  inline constexpr vector_t& operator[](usz i) { return vs[i]; }
  inline constexpr vector_t const& operator[](usz i) const { return vs[i]; }
  inline constexpr T* data() { return elements; }
  inline constexpr T const* data() const { return elements; }

  constexpr explicit MatrixT(MatrixT<T, N + 1> const& m)
  requires (N == 3)
  : elements{m[0][0], m[0][1], m[0][2],
             m[1][0], m[1][1], m[1][2],
             m[2][0], m[2][1], m[2][2]} { }

  static constexpr mat Identity()
  requires (N == 3)
  {
      return {
        1, 0, 0,
        0, 1, 0,
        0, 0, 1
      };
  }

  static constexpr mat Identity()
  requires (N == 4)
  {
      return {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
      };
  }

  static constexpr mat Identity()
  {
    mat result;
    for (usz i = 0; i < N; ++i)
      result[i][i] = 1;
    return result;
  }

  inline constexpr mat transpose() const
  requires (N == 3)
  {
    return {
      elements[0], elements[3], elements[6],
      elements[1], elements[4], elements[7],
      elements[2], elements[5], elements[8]
    };
  }

  inline constexpr mat transpose() const
  requires (N == 4)
  {
    return {
      elements[0], elements[4], elements[8],  elements[12],
      elements[1], elements[5], elements[9],  elements[13],
      elements[2], elements[6], elements[10], elements[14],
      elements[3], elements[7], elements[11], elements[15]
    };
  }

  inline constexpr mat transpose() const
  {
    MatrixT<T, N> result;
    for (usz i = 0; i < N; ++i)
      for (usz j = 0; j < N; ++j)
        result[i][j] = elements[i + j * N];
    return result;
  }

  inline constexpr mat inverse() const
  requires (N == 3)
  {
    mat result = Identity();
    T const* e = elements;
    T const det =
      e[0] * (e[4] * e[8] - e[5] * e[7])
    - e[1] * (e[3] * e[8] - e[5] * e[6])
    + e[2] * (e[3] * e[7] - e[4] * e[6]);

    if (det != 0.f)
    {
      T* r = result.elements;
      T const i = 1.f / det;
      r[0] =  (e[4] * e[8] - e[5] * e[7]) * i;
      r[1] = -(e[1] * e[8] - e[2] * e[7]) * i;
      r[2] =  (e[1] * e[5] - e[2] * e[4]) * i;

      r[3] = -(e[3] * e[8] - e[5] * e[6]) * i;
      r[4] =  (e[0] * e[8] - e[2] * e[6]) * i;
      r[5] = -(e[0] * e[5] - e[2] * e[3]) * i;

      r[6] =  (e[3] * e[7] - e[4] * e[6]) * i;
      r[7] = -(e[0] * e[7] - e[1] * e[6]) * i;
      r[8] =  (e[0] * e[4] - e[1] * e[3]) * i;
    }
    return result;
  }

  inline constexpr mat inverse() const
  requires (N == 4)
  {
    mat result = Identity();
    T const* e = elements;
    T s[6];
    T c[6];

    s[0] = e[0] * e[5] - e[4] * e[1];
    s[1] = e[0] * e[6] - e[4] * e[2];
    s[2] = e[0] * e[7] - e[4] * e[3];
    s[3] = e[1] * e[6] - e[5] * e[2];
    s[4] = e[1] * e[7] - e[5] * e[3];
    s[5] = e[2] * e[7] - e[6] * e[3];

    c[0] = e[8]  * e[13] - e[12] * e[9];
    c[1] = e[8]  * e[14] - e[12] * e[10];
    c[2] = e[8]  * e[15] - e[12] * e[11];
    c[3] = e[9]  * e[14] - e[13] * e[10];
    c[4] = e[9]  * e[15] - e[13] * e[11];
    c[5] = e[10] * e[15] - e[14] * e[11];

    // assume it is invertivle
    T idet = 1.0/(s[0]*c[5]-s[1]*c[4]+s[2]*c[3]+s[3]*c[2]-s[4]*c[1]+s[5]*c[0]);

    T* r = result.elements;
    r[0]  = ( e[5]  * c[5] - e[6]  * c[4] + e[7]  * c[3]) * idet;
    r[1]  = (-e[1]  * c[5] + e[2]  * c[4] - e[3]  * c[3]) * idet;
    r[2]  = ( e[13] * s[5] - e[14] * s[4] + e[15] * s[3]) * idet;
    r[3]  = (-e[9]  * s[5] + e[10] * s[4] - e[11] * s[3]) * idet;

    r[4]  = (-e[4]  * c[5] + e[6]  * c[2] - e[7]  * c[1]) * idet;
    r[5]  = ( e[0]  * c[5] - e[2]  * c[2] + e[3]  * c[1]) * idet;
    r[6]  = (-e[12] * s[5] + e[14] * s[2] - e[15] * s[1]) * idet;
    r[7]  = ( e[8]  * s[5] - e[10] * s[2] + e[11] * s[1]) * idet;

    r[8]  = ( e[4]  * c[4] - e[5]  * c[2] + e[7]  * c[0]) * idet;
    r[9]  = (-e[0]  * c[4] + e[1]  * c[2] - e[3]  * c[0]) * idet;
    r[10] = ( e[12] * s[4] - e[13] * s[2] + e[15] * s[0]) * idet;
    r[11] = (-e[8]  * s[4] + e[9]  * s[2] - e[11] * s[0]) * idet;

    r[12] = (-e[4]  * c[3] + e[5]  * c[1] - e[6]  * c[0]) * idet;
    r[13] = ( e[0]  * c[3] - e[1]  * c[1] + e[2]  * c[0]) * idet;
    r[14] = (-e[12] * s[3] + e[13] * s[1] - e[14] * s[0]) * idet;
    r[15] = ( e[8]  * s[3] - e[9]  * s[1] + e[10] * s[0]) * idet;
    return result;
  }

  inline constexpr vector_t operator*(vector_t const& v)
  requires (N == 3)
  {
    mat tm = transpose();
    return {
      dotProduct(tm[0], v),
      dotProduct(tm[1], v),
      dotProduct(tm[2], v)
    };
  }

  inline constexpr vector_t operator*(vector_t const& v)
  requires (N == 4)
  {
    mat tm = transpose();
    return {
      dotProduct(tm[0], v),
      dotProduct(tm[1], v),
      dotProduct(tm[2], v),
      dotProduct(tm[3], v)
    };
  }

  inline constexpr vector_t operator*(vector_t const& v)
  {
    mat tm = transpose();
    vector_t result;
    for (usz i = 0; i < N; ++i)
      result[i] = dotProduct(tm[i], v);
    return result;
  }

  inline constexpr VectorT<T, N - 1>
  transformNormal(VectorT<T, N - 1> const& v)
  {
    vector_t result = operator*(VectorT<T, N>(v, 0));
    return VectorT<T, N-1>(result);
  }

  inline constexpr VectorT<T, N - 1>
  transform(VectorT<T, N - 1> const& v)
  {
    vector_t result = operator*(VectorT<T, N>(v, 1));
    return VectorT<T, N-1>(result) / result[N-1];
  }

  inline constexpr mat operator*(mat const& r)
  {
    mat temp(*this);
    return temp.combine(r);
  }

  inline constexpr mat& combine(mat const& r)
  {
    mat temp = transpose();
    for (usz i = 0; i < N; ++i)
      for (usz j = 0; j < N; ++j)
        vs[i][j] = dotProduct(temp[j], r[i]);
    return *this;
  }

  inline constexpr mat& translate(VectorT<T, N - 1> const& tr)
  requires (N == 3)
  {
    return combine(
      mat(1, 0, 0,
          0, 1, 0,
          tr.x, tr.y, 1)
    );
  }

  inline constexpr mat& translate(VectorT<T, N - 1> const& tr)
  requires (N == 4)
  {
    return combine(
        mat(1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            tr.x, tr.y, tr.z, 1)
        );
  }

  template <Arithmetic ... Ts>
  inline constexpr mat& translate(Ts... values)
  requires ((N == 4 || N == 3) && sizeof...(Ts) == N - 1)
  {
    return translate(VectorT<T, N - 1>(values...));
  }

  inline constexpr mat& scale(VectorT<T, N - 1> const& sc)
  requires (N == 3)
  {
    return combine(
        mat(sc.x, 0, 0,
            0, sc.y, 0,
            0, 0, 1)
        );
  }

  inline constexpr mat& scale(VectorT<T, N - 1> const& sc)
  requires (N == 4)
  {
    return combine(
        mat(sc.x, 0, 0, 0,
            0, sc.y, 0, 0,
            0, 0, sc.z, 0,
            0, 0, 0, 1)
        );
  }

  template <Arithmetic ... Ts>
  inline constexpr mat& scale(Ts... values)
  requires ((N == 4 || N == 3) && sizeof...(Ts) == N - 1)
  {
    return scale(VectorT<T, N - 1>(values...));
  }

  inline constexpr mat& rotate(Angle th, VectorT<T, N - 1> const& axis = VectorT<T, N - 1>(0, 0))
  requires (N == 3)
  {
    T const s = th.sin();
    T const c = th.cos();
    if (isZero(axis))
    {
      return combine(
          mat(c, s, 0,
             -s, c, 0,
              0, 0, 1)
          );
    }
    T tx = - axis.x * c + axis.y * s + axis.x;
    T ty = - axis.x * s - axis.y * c + axis.y;
    return combine(
        mat(c, s, 0,
           -s, c, 0,
           tx, ty, 1)
        );
  }

  inline constexpr mat& rotate(Angle th, VectorT<T, N - 1> const& axis)
  requires (N == 4)
  {
    T const s = th.sin();
    T const c = th.cos();
    auto a = normalize(axis);
    auto aa = (1 - c) * a;
    return combine(
        mat(
      c + a.x * aa.x,       a.y * aa.x + a.z * s, a.z * aa.x - a.y * s, 0,
      a.x * aa.y - a.z * s, c + a.y * aa.y,       a.z * aa.y + a.x * s, 0,
      a.x * aa.z + a.y * s, a.y * aa.z - a.x * s, c + a.z * aa.z,       0,
      0,                    0,                    0,                    1
    ));
  }

  static inline constexpr mat ortho(T left, T right, T top, T bottom, T near, T far)
  requires (N == 4)
  {
    mat projection;
    T const two = static_cast<T>(2);
    projection.elements[0]  =   two / (right - left);
    projection.elements[5]  =   two / (top - bottom);
    projection.elements[10] = - two / (far - near);
    projection.elements[12] = - (right + left) / (right - left);
    projection.elements[13] = - (top + bottom) / (top - bottom);
    projection.elements[14] = - (far + near)   / (far - near);
    projection.elements[15] = 1;
    return projection;
  }

  static inline constexpr mat perspective(Angle yFov, T aspectRatio, T near, T far)
  requires (N == 4)
  {
    mat projection;
    T const tan_half = (yFov / static_cast<T>(2)).tan();
    T const one = static_cast<T>(1);
    projection.elements[0]  =   one / (aspectRatio * tan_half);
    projection.elements[5]  =   one / tan_half;
    projection.elements[10] =   far / (near - far);
    projection.elements[11] = - one;
    projection.elements[14] = - (static_cast<T>(2) * far * near) / (far - near);
    // projection.elements[15] = 0;
    return projection;
  }

  static inline constexpr mat lookAt(VectorT<T, 3> const& camera,
                                     VectorT<T, 3> const& target,
                                     VectorT<T, 3> const& up)
  requires (N == 4)
  {
    auto const f = normalize((camera - target));
    auto const s = normalize(crossProduct(f, up));
    auto const t = crossProduct(s, f);
    return mat(
          s.x, t.x, f.x, 0,
          s.y, t.y, f.y, 0,
          s.z, t.z, f.z, 0,
          - dotProduct(s, camera),
          - dotProduct(t, camera),
          - dotProduct(f, camera),
          1
        );
  }

  bool inline constexpr operator==(MatrixT<T, N> const& rhs) const
  requires (Integral<T>)
  {
    for (usz i = 0; i < N; ++i)
      for (usz j = 0; j < N; ++j)
        if (rhs.elements[j + i * N] != elements[j + i * N])
          return false;
    return true;
  }

  bool inline constexpr operator==(MatrixT<T, N> const& rhs) const
  requires (FloatingPoint<T>)
  {
    for (usz i = 0; i < N; ++i)
      for (usz j = 0; j < N; ++j)
        if (isZero(rhs.elements[j + i * N] - elements[j + i * N]) == false)
          return false;
    return true;
  }

private:
  union
  {
    value_t elements[N * N];
    vector_t vs[N];
  };
};

} // namespace math

#endif // MATRIX_HPP_
