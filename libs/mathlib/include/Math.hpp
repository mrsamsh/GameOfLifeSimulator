//
// Math.hpp
// mathlib
//
// Created by Usama Alshughry 24.07.2024.
// Copyright © 2024 Usama Alshughry. All rights reserved.
//

#ifndef MATH_HPP_
#define MATH_HPP_

#include "MyTypes.hpp"
#include "Angle.hpp"
#include "Vector.hpp"
#include "Matrix.hpp"
#include "Quaternion.hpp"
#include "Random.hpp"
#include "Shape.hpp"

namespace math
{

using vec2  = VectorT<f32, 2>;
using vec3  = VectorT<f32, 3>;
using vec4  = VectorT<f32, 4>;

using dvec2 = VectorT<f64, 2>;
using dvec3 = VectorT<f64, 3>;
using dvec4 = VectorT<f64, 4>;

using ivec2 = VectorT<i32, 2>;
using ivec3 = VectorT<i32, 3>;
using ivec4 = VectorT<i32, 4>;

using uvec2 = VectorT<u32, 2>;
using uvec3 = VectorT<u32, 3>;
using uvec4 = VectorT<u32, 4>;

using mat4  = MatrixT<f32, 4>;
using dmat4 = MatrixT<f64, 4>;

using versor = QuaternionT<f32>;

using Rect  = RectT<f32>;
using iRect = RectT<i32>;

using Circle  = CircleT<f32>;
using iCircle = CircleT<i32>;

using Ellipse  = EllipseT<f32>;
using iEllipse = EllipseT<i32>;

} // namespace math

#endif // MATH_HPP_
