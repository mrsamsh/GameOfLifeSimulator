//
// main.cpp
// mathlib
//
// Created by Usama Alshughry 19.07.2024.
// Copyright © 2024 Usama Alshughry. All rights reserved.
//

#include <iostream>
#include <print>
#include "MathPrint.hpp"
#include "Math.hpp"
#include "Clock.hpp"

#define test(x) if (!(x)) { printf("Failed on line %d\n", __LINE__); exit(-1); }

int main()
{
  using std::println;

  using namespace math;
  using test_t = f64;
  using vec2 = VectorT<test_t, 2>;
  using vec3 = VectorT<test_t, 3>;
  using vec4 = VectorT<test_t, 4>;
  using mat3 = MatrixT<test_t, 3>;
  using mat4 = MatrixT<test_t, 4>;

  vec2::value_t x1 = 1.004;
  vec2::value_t y1 = -2.99334;
  vec2::value_t x2 = 100.2994;
  vec2::value_t y2 = -14.3001;
  vec2::value_t z1 = -0.0002;
  vec2::value_t z2 = 14567634;
  vec2::value_t w1 = 9.22;
  vec2::value_t w2 = M_PI;

  {
    vec2 v1(x1, y1);
    vec2 v2(x2, y2);

    test((v1+v2).x == x1+x2);
    test((v1+v2).y == y1+y2);

    test((v1-v2).x == x1-x2);
    test((v1-v2).y == y1-y2);

    test((v1*v2).x == x1*x2);
    test((v1*v2).y == y1*y2);

    test((v1/v2).x == x1/x2);
    test((v1/v2).y == y1/y2);

    test(v1 * x1 == vec2(x1 * x1, y1 * x1));
    test(v1 / x1 == vec2(x1 / x1, y1 / x1));

    test(v2 * x1 == vec2(x2 * x1, y2 * x1));

    {
      test(v2 / x1 == vec2(x2 / x1, y2 / x1));
      test(v2 / y1 == vec2(x2 / y1, y2 / y1));
    }

    test(length2(v1) == x1 * x1 + y1 * y1);
    test(length(v1) == std::sqrt(x1 * x1 + y1 * y1));
    test(length(v2) == std::sqrt(x2 * x2 + y2 * y2));

    {
      vec2::value_t scalar = length(v1);
      test(normalize(v1) == vec2(v1.x / scalar, v1.y / scalar));
    }
    test(math::dotProduct(v1, v2) == x1 * x2 + y1 * y2);
  }

  {
    vec3 v1(x1, y1, z1);
    vec3 v2(x2, y2, z2);

    test((v1+v2).x == x1+x2);
    test((v1+v2).y == y1+y2);
    test((v1+v2).z == z1+z2);

    test((v1-v2).x == x1-x2);
    test((v1-v2).y == y1-y2);
    test((v1-v2).z == z1-z2);

    test((v1*v2).x == x1*x2);
    test((v1*v2).y == y1*y2);
    test((v1*v2).z == z1*z2);

    test((v1/v2).x == x1/x2);
    test((v1/v2).y == y1/y2);
    test((v1/v2).z == z1/z2);

    test(v1 * x1 == vec3(x1 * x1, y1 * x1, z1 * x1));
    {
      test(v1 / x1 == vec3(x1 / x1, y1 / x1, z1 / x1));
    }

    test(v2 * x1 == vec3(x2 * x1, y2 * x1, z2 * x1));

    {
      test(v2 / x1 == vec3(x2 / x1, y2 / x1, z2 / x1));
      test(v2 / y1 == vec3(x2 /y1, y2 /y1, z2 /y1));
    }

    test(length2(v1) == x1 * x1 + y1 * y1 + z1 * z1);
    test(length(v1) == std::sqrt(x1 * x1 + y1 * y1 + z1 * z1));
    test(length(v2) == std::sqrt(x2 * x2 + y2 * y2 + z2 * z2));

    {
      vec3::value_t scalar = length(v1);
      test(normalize(v1) == vec3(v1.x / scalar, v1.y / scalar, v1.z / scalar));
    }

    test(math::dotProduct(v1, v2) == x1 * x2 + y1 * y2 + z1 * z2);
  }

  {
    vec4 v1(x1, y1, z1, w1);
    vec4 v2(x2, y2, z2, w2);

    test((v1+v2).x == x1+x2);
    test((v1+v2).y == y1+y2);
    test((v1+v2).z == z1+z2);
    test((v1+v2).w == w1+w2);

    test((v1-v2).x == x1-x2);
    test((v1-v2).y == y1-y2);
    test((v1-v2).z == z1-z2);
    test((v1-v2).w == w1-w2);

    test((v1*v2).x == x1*x2);
    test((v1*v2).y == y1*y2);
    test((v1*v2).z == z1*z2);
    test((v1*v2).w == w1*w2);

    test((v1/v2).x == x1/x2);
    test((v1/v2).y == y1/y2);
    test((v1/v2).z == z1/z2);
    test((v1/v2).w == w1/w2);

    test(v1 * x1 == vec4(x1 * x1, y1 * x1, z1 * x1, w1 * x1));
    {
      test(v1 / x1 == vec4(x1 / x1, y1 / x1, z1 / x1, w1 / x1));
    }

    test(v2 * x1 == vec4(x2 * x1, y2 * x1, z2 * x1, w2 * x1));

    {
      test(v2 / x1 == vec4(x2 / x1, y2 / x1, z2 / x1, w2 / x1));
      test(v2 / y1 == vec4(x2 / y1, y2 / y1, z2 / y1, w2 / y1));
    }

    test(length2(v1) == x1 * x1 + y1 * y1 + z1 * z1 + w1 * w1);
    test(length(v1) == std::sqrt(x1 * x1 + y1 * y1 + z1 * z1 + w1 * w1));
    test(length(v2) == std::sqrt(x2 * x2 + y2 * y2 + z2 * z2 + w2 * w2));

    {
      vec4::value_t scalar =  length(v1);
      test(normalize(v1) == vec4(v1.x / scalar, v1.y / scalar, v1.z / scalar, v1.w / scalar));
    }

    test(math::dotProduct(v1, v2) == x1 * x2 + y1 * y2 + z1 * z2 + w1 * w2);
  }
  {
    mat3 m(1, 0, 0,
           0, 2, 1,
           0, 0, 1);

    vec3 v(1, -5, 0);
    vec3 r = m * v;
    test(r == vec3(1, -10, -5));
  }
  {

    mat4 m = mat4::Identity();
    vec3 trans(10, 10, 1);
    m.translate(trans);
    m.rotate(90_deg, {0, 0, 1});
    m.scale(2, 2, 1);
    vec4 v{1, 12, 11, 1};
    vec4 r = m * v;

    test(r == vec4(-14, 12, 12, 1));

    Rect r1{10, 10, 100, 100};
    Rect r2{110, 15, 19, 11};
    test(pointInside(r1, {20, 20}) == true);
    Circle c1{0, 0, 15};
    Circle c2{20, 0, 5};
    test(checkIntersection(r1, r1) == true);
    test(checkIntersection(c1, r1) == true);
    test(checkIntersection(c1, c2) == true);
    mat3 m1 = mat3::Identity();
    m1.translate(10, 14);
    m1.rotate(90_deg);
    m1.scale(2, 4);
    mat3 im1 = m1.inverse();
    test(m1 * im1 == mat3::Identity());
    test(m * m.inverse() == mat4::Identity());
  }
  math::VectorT<f32, 9> v9(1, 2, 3, 4, 5, 6, 7, 8, 9);
  math::VectorT<f32, 7> v7(v9);
  math::VectorT<f32, 12> v12(v7);
  println("{}\n{}\n{}", v9, v7, v12);
  math::VectorT<f32, 2> vv(v9);
  math::VectorT<f32, 3> vvv(v9);
  math::VectorT<f32, 4> vvvv(v9);
  println("{}\n{}\n{}", vv, vvv, vvvv);
}
