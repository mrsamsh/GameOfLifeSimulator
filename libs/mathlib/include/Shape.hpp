//
// Shape.hpp
// mathlib
//
// Created by Usama Alshughry 26.07.2024.
// Copyright © 2024 Usama Alshughry. All rights reserved.
//

#ifndef SHAPE_HPP_
#define SHAPE_HPP_

#include "MathConcepts.hpp"
#include "MathHelper.hpp"
#include "Vector.hpp"

namespace math
{

template <Arithmetic T>
struct CircleT
{
  [[nodiscard]] constexpr CircleT() : center{}, radius{} { }
  [[nodiscard]] constexpr CircleT(VectorT<T, 2> const& center, T const radius)
    : center{center}, radius{radius} { }
  [[nodiscard]] constexpr CircleT(T const a, T const b, T const c)
    : x{a}, y{b}, radius{c} { }

  union { VectorT<T, 2> center; struct { T x, y; }; };
  T radius;
};

template <Arithmetic T>
struct EllipseT
{
  [[nodiscard]] constexpr EllipseT() : center{}, rx{}, ry{} { }
  [[nodiscard]] constexpr EllipseT(VectorT<T, 2> const& center, T const rx, T const ry)
    : center{center}, rx{rx}, ry{ry} { }
  [[nodiscard]] constexpr EllipseT(T const a, T const b, T const c, T const d)
    : x{a}, y{b}, rx{c}, ry{d} { }

  union { VectorT<T, 2> center; struct { T x, y; }; };
  T rx, ry;
};

template <Arithmetic T>
struct RectT
{
  [[nodiscard]] constexpr RectT() : x(0), y(0), w(0), h(0) { }
  [[nodiscard]] constexpr RectT(VectorT<T, 2> const& pos, VectorT<T, 2> const& size)
    : position{pos}, size{size} { }
  [[nodiscard]] constexpr RectT(T const a, T const b, T const c, T const d)
    : x{a}, y{b}, w{c}, h{d} { }

  inline constexpr T left()   const { return min(x, x + w); }
  inline constexpr T right()  const { return max(x, x + w); }
  inline constexpr T top()    const { return min(y, y + h); }
  inline constexpr T bottom() const { return max(y, y + h); }

  union { VectorT<T, 2> position; struct { T x, y; }; };
  union { VectorT<T, 2> size;     struct { T w, h; }; };
};

template <Arithmetic T>
struct SphereT
{
  [[nodiscard]] constexpr SphereT() : x(0), y(0), z(0), radius(0) { }
  [[nodiscard]] constexpr SphereT(VectorT<T, 3> const& center, T const radius)
    : center{center}, radius{radius} { }
  [[nodiscard]] constexpr SphereT(T const a, T const b, T const c, T const d)
    : x{a}, y{b}, z{c}, radius{d} { }

  union { VectorT<T, 3> center; struct { T x, y, z;}; };
  T radius;
};

template <Arithmetic T>
struct BoxT
{
  [[nodiscard]] constexpr BoxT() : x(0), y(0), z(0), w(0), h(0), d(0) { }
  [[nodiscard]] constexpr BoxT(VectorT<T, 3> const& pos, VectorT<T, 3> const& size)
    : position{pos}, size{size} { }
  [[nodiscard]] constexpr BoxT(T a, T b, T c, T d, T e, T f)
    : x{a}, y{b}, z{c}, w{d}, h{e}, d{f} { }

  inline constexpr T left()   const { return min(x, x + w); }
  inline constexpr T right()  const { return max(x, x + w); }
  inline constexpr T bottom() const { return min(y, y + h); }
  inline constexpr T top()    const { return max(y, y + h); }
  inline constexpr T far()    const { return min(z, z + d); }
  inline constexpr T near()   const { return max(z, z + d); }

  union { VectorT<T, 3> position; struct { T x, y, z; }; };
  union { VectorT<T, 3> size;     struct { T w, h, d; }; };
};

template <Arithmetic T>
inline constexpr bool
pointInside(RectT<T> const& r, VectorT<T, 2> const& p)
{
  return p.x >= r.left() && p.x <= r.right()
      && p.y >= r.top()  && p.y <= r.bottom();
}

template <Arithmetic T>
inline constexpr bool
pointInside(CircleT<T> const& c, VectorT<T, 2> const& p)
{
  return length2(p - c.center) <= (c.radius * c.radius);
}

template <Arithmetic T>
inline constexpr bool
pointInside(SphereT<T> const& s, VectorT<T, 3> const& p)
{
  return length2(s.center - p) <= (s.radius * s.radius);
}

template <Arithmetic T>
inline constexpr bool
pointInside(BoxT<T> const& b, VectorT<T, 3> const& p)
{
  return p.x >= b.left()   && p.x <= b.right()
      && p.y >= b.bottom() && p.y <= b.top()
      && p.z >= b.far()    && p.z <= b.near();
}

template <Arithmetic T>
inline constexpr RectT<T>
calculateMinkowski(RectT<T> const& r1, RectT<T> const& r2)
{
  return {
    r1.left() - r2.right(),
    r1.top()  - r2.bottom(),
    abs(r1.w) + abs(r2.w),
    abs(r1.h) + abs(r2.h)
  };
}

template <Arithmetic T>
inline constexpr BoxT<T>
calculateMinkowski(BoxT<T> const& b1, BoxT<T> const& b2)
{
  return {
    b1.left() - b2.right(),
    b1.bottom()  - b2.top(),
    b1.far()  - b2.near(),
    abs(b1.w) + abs(b2.w),
    abs(b1.h) + abs(b2.h),
    abs(b1.d) + abs(b2.d)
  };
}

template <Arithmetic T>
inline constexpr bool
checkIntersection(RectT<T> const& r, CircleT<T> const& c)
{
  VectorT<T, 2> nearest{
    clamp(c.x, r.left(), r.right()),
    clamp(c.y, r.top(), r.bottom())
  };
  return pointInside(c, nearest);
}

template <Arithmetic T>
inline constexpr bool
checkIntersection(CircleT<T> const& c, RectT<T> const& r)
{
  return checkIntersection(r, c);
}

template <Arithmetic T>
inline constexpr bool
checkIntersection(RectT<T> const& r1, RectT<T> const& r2)
{
  return pointInside(calculateMinkowski(r1, r2), {0, 0});
}

template <Arithmetic T>
inline constexpr bool
checkIntersection(BoxT<T> const& b1, BoxT<T> const& b2)
{
  return pointInside(calculateMinkowski(b1, b2), {0, 0, 0});
}

template <Arithmetic T>
inline constexpr bool
checkIntersection(CircleT<T> const& c1, CircleT<T> const& c2)
{
  return length2(c1.center - c2.center) <= (c1.radius + c2.radius) * (c1.radius + c2.radius);
}

} // namespace math

#endif // SHAPE_HPP_
