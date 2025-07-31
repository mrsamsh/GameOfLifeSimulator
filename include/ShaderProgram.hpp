//
// ShaderProgram.hpp
// OpenGLBP
//
// Created by Usama Alshughry 13.07.2024.
// Copyright © 2024 Usama Alshughry. All rights reserved.
//

#ifndef SHADERPROGRAM_HPP_
#define SHADERPROGRAM_HPP_

#include <vector>
#include <string_view>
#include <glad/glad.h>
#include <concepts>
#include <typeinfo>
#include "Log.hpp"
#include "Vector.hpp"
#include "ShaderUniformHelper.hpp"

class ShaderProgram
{
public:
  void loadProgram(std::vector<std::pair<GLenum, std::string_view>> shaderList);
  void loadProgramFromString(std::vector<std::pair<GLenum, std::string_view>> shaderList);
  ShaderProgram& use();
  GLuint name() const;

  template <typename Value, typename ... Args>
  requires (std::same_as<Value, Args> && ...)
  ShaderProgram& set(std::string_view name, Value value, Args&& ... args)
  {
    auto location = getLocation(name);
    UniformHelper<Value, 1 + sizeof...(Args)>::setter(location, value, std::forward<Args>(args)...);
    return *this;
  }

  template <typename Value, size_t N>
  ShaderProgram& set(std::string_view name, math::VectorT<Value, N> const& value)
  {
    auto location = getLocation(name);
    UniformHelper<math::VectorT<Value, N>, N>::setter(location, 1, value.data());
    return *this;
  }

  template <typename Value, size_t N>
  ShaderProgram& set(std::string_view name,
      math::MatrixT<Value, N> const& value, bool transpose = false)
  {
    auto location = getLocation(name);
    UniformHelper<math::MatrixT<Value, N>, N>
      ::setter(location, 1, transpose, value.data());
    return *this;
  }

private:
  GLint getLocation(std::string_view name);
  GLuint id = 0;
};

#endif // SHADERPROGRAM_HPP_
