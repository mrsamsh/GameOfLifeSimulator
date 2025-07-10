//
// ShaderUniformHelper.hpp
// OpenGLBP
//
// Created by Usama Alshughry 13.07.2024.
// Copyright © 2024 Usama Alshughry. All rights reserved.
//

#ifndef SHADERUNIFORMHELPER_HPP_
#define SHADERUNIFORMHELPER_HPP_

#include <cstddef>
#include <glad/glad.h>
#include "Vector.hpp"
#include "Matrix.hpp"

template <typename Value, size_t N>
struct UniformHelper;

template <>
struct UniformHelper<GLint, 1>
{
  static constexpr auto& setter = glUniform1i;
};
template <>
struct UniformHelper<GLint, 2>
{
  static constexpr auto& setter = glUniform2i;
};
template <>
struct UniformHelper<GLint, 3>
{
  static constexpr auto& setter = glUniform3i;
};
template <>
struct UniformHelper<GLint, 4>
{
  static constexpr auto& setter = glUniform4i;
};

template <>
struct UniformHelper<GLuint, 1>
{
  static constexpr auto& setter = glUniform1ui;
};
template <>
struct UniformHelper<GLuint, 2>
{
  static constexpr auto& setter = glUniform2ui;
};
template <>
struct UniformHelper<GLuint, 3>
{
  static constexpr auto& setter = glUniform3ui;
};
template <>
struct UniformHelper<GLuint, 4>
{
  static constexpr auto& setter = glUniform4ui;
};

template <>
struct UniformHelper<GLfloat, 1>
{
  static constexpr auto& setter = glUniform1f;
};
template <>
struct UniformHelper<GLfloat, 2>
{
  static constexpr auto& setter = glUniform2f;
};
template <>
struct UniformHelper<GLfloat, 3>
{
  static constexpr auto& setter = glUniform3f;
};
template <>
struct UniformHelper<GLfloat, 4>
{
  static constexpr auto& setter = glUniform4f;
};

template <>
struct UniformHelper<bool, 1>
{
  static constexpr auto& setter = glUniform1i;
};
template <>
struct UniformHelper<bool, 2>
{
  static constexpr auto& setter = glUniform2i;
};
template <>
struct UniformHelper<bool, 3>
{
  static constexpr auto& setter = glUniform3i;
};
template <>
struct UniformHelper<bool, 4>
{
  static constexpr auto& setter = glUniform4i;
};

template <>
struct UniformHelper<math::VectorT<GLfloat, 1>, 1>
{
  static constexpr auto& setter = glUniform1fv;
};
template <>
struct UniformHelper<math::VectorT<GLfloat, 2>, 2>
{
  static constexpr auto& setter = glUniform2fv;
};
template <>
struct UniformHelper<math::VectorT<GLfloat, 3>, 3>
{
  static constexpr auto& setter = glUniform3fv;
};
template <>
struct UniformHelper<math::VectorT<GLfloat, 4>, 4>
{
  static constexpr auto& setter = glUniform4fv;
};

template <>
struct UniformHelper<math::MatrixT<GLfloat, 2>, 2>
{
  static constexpr auto& setter = glUniformMatrix2fv;
};
template <>
struct UniformHelper<math::MatrixT<GLfloat, 3>, 3>
{
  static constexpr auto& setter = glUniformMatrix3fv;
};
template <>
struct UniformHelper<math::MatrixT<GLfloat, 4>, 4>
{
  static constexpr auto& setter = glUniformMatrix4fv;
};

#endif // SHADERUNIFORMHELPER_HPP_
