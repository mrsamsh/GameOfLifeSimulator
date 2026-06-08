//
// MathPrint.hpp
// mathlib
//
// Created by Usama Alshughry 20.07.2024.
// Copyright © 2024 Usama Alshughry. All rights reserved.
//

#ifndef MATHPRINT_HPP_
#define MATHPRINT_HPP_

#include <concepts>
#include <format>

#include "Angle.hpp"
#include "Vector.hpp"
#include "Matrix.hpp"
#include "Time.hpp"
#include "Quaternion.hpp"
#include "Shape.hpp"

template <>
struct std::formatter<math::Time>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(math::Time const& t, FormatContext& ctx) const
  {
    return std::format_to(ctx.out(), "{: >8.3f} sec", t.asSeconds());
  }
};

template <>
struct std::formatter<math::Angle>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(math::Angle const& th, FormatContext& ctx) const
  {
    return std::format_to(ctx.out(), "{: >8.3f} deg", th.asDegrees());
  }
};

template <math::Arithmetic T, usz N>
struct std::formatter<math::VectorT<T, N>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(math::VectorT<T, N> const& v, FormatContext& ctx) const
  {
    if constexpr (math::FloatingPoint<T>)
    {
      std::format_to(ctx.out(), "v{}[{: >8.3f}, ", N, v[0]);
      for (usz i = 1; i < N - 1; ++i)
        std::format_to(ctx.out(), "{: >8.3f}, ", v[i]);
      return std::format_to(ctx.out(), "{: >8.3f}]", v[N-1]);
    }
    else
    {
      std::format_to(ctx.out(), "v{}[{: >4}, ", N, v[0]);
      for (usz i = 1; i < N - 1; ++i)
        std::format_to(ctx.out(), "{: >4}, ", v[i]);
      return std::format_to(ctx.out(), "{: >4}]", v[N-1]);
    }
  }
};

template <math::Arithmetic T, usz N>
struct std::formatter<math::MatrixT<T, N>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(math::MatrixT<T, N> const& m, FormatContext& ctx) const
  {
    for (usz i = 0; i < N - 1; ++i)
      std::format_to(ctx.out(), "m{}\n",  m[i]);
    return std::format_to(ctx.out(), "m{}",  m[N-1]);
  }
};

template <math::FloatingPoint T>
struct std::formatter<math::QuaternionT<T>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(math::QuaternionT<T> const& q, FormatContext& ctx) const
  {
    return std::format_to(ctx.out(), "q[{: >8.3f}, {}]", q.w, q.axis);
  }
};

template <math::Arithmetic T>
struct std::formatter<math::RectT<T>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(math::RectT<T> const& r, FormatContext& ctx) const
  {
    return std::format_to(ctx.out(), "Rect [position: {}, size: {}]", r.position, r.size);
  }
};

template <math::Arithmetic T>
struct std::formatter<math::CircleT<T>>
{
  template <typename ParseContext>
  constexpr auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(math::CircleT<T> const& c, FormatContext& ctx) const
  {
    return std::format_to(ctx.out(), "Circle [center: {}, rad: {}]", c.center, c.radius);
  }
};

#endif // MATHPRINT_HPP_
