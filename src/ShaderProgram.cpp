//
// ShaderProgram.cpp
// OpenGLBP
//
// Created by Usama Alshughry 13.07.2024.
// Copyright © 2024 Usama Alshughry. All rights reserved.
//

#include "ShaderProgram.hpp"
#include "File.hpp"
#include "Log.hpp"
#include "Vector.hpp"

namespace priv
{

enum class StatusFor
{
  Program,
  Shader
};

static inline constexpr
std::string_view getShaderStr(GLenum type)
{
  switch (type)
  {
    case GL_VERTEX_SHADER:
      return "Vertex Shader";
    case GL_FRAGMENT_SHADER:
      return "Fragment Shader";
    case GL_TESS_CONTROL_SHADER:
      return "Tesselation Control Shader";
    case GL_TESS_EVALUATION_SHADER:
      return "Tesselation Evaluation Shader";
    case GL_GEOMETRY_SHADER:
      return "Geometry Shader";
    // case GL_COMPUTE_SHADER:
    //   return "Compute Shader";
    default:
      return "Unknown type";
  }
}

template <StatusFor S>
struct StatusHelper;

template<>
struct StatusHelper<StatusFor::Shader>
{
  static constexpr auto& infoLog = glGetShaderInfoLog;
  static constexpr auto& success = glGetShaderiv;
  static constexpr auto status = GL_COMPILE_STATUS;
};

template<>
struct StatusHelper<StatusFor::Program>
{
  static constexpr auto& infoLog = glGetProgramInfoLog;
  static constexpr auto& success = glGetProgramiv;
  static constexpr auto status = GL_LINK_STATUS;
};

template <StatusFor S>
inline
std::string_view getInfoLog(GLuint name)
{
  static usz const MaxSize = 512;
  static char buffer[MaxSize];
  StatusHelper<S>::infoLog(name, MaxSize, 0, buffer);
  return buffer;
}

template <StatusFor S>
inline constexpr
bool hasFailed(GLuint name)
{
  i32 result;
  StatusHelper<S>::success(name, StatusHelper<S>::status, &result);
  return result == 0;
}

GLuint loadShader(GLenum type, std::string_view filename)
{
  GLuint shader = glCreateShader(type);
  auto str = bp::loadFile(filename);
  char const* source = nullptr;
  if (str)
  {
    source = (*str).data();
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);
  }
  if (hasFailed<StatusFor::Shader>(shader))
  {
    log::println("Failed to compile {} shader with source\n{}\n error: {}",
        priv::getShaderStr(type),
        (*str).data(),
        getInfoLog<StatusFor::Shader>(shader));
  }
  return shader;
}

} // namespace priv

void ShaderProgram::loadProgram(std::vector<std::pair<GLenum, std::string_view>> shaderList)
{
  id = glCreateProgram();
  std::vector<GLuint> loadedShaders;
  for (auto& [type, filename] : shaderList)
  {
    auto shader = priv::loadShader(type, filename);
    glAttachShader(id, shader);
    loadedShaders.push_back(shader);
  }

  glLinkProgram(id);
  if (priv::hasFailed<priv::StatusFor::Program>(id))
  {
    log::println("Failed to link program: {}",
        priv::getInfoLog<priv::StatusFor::Program>(id));
  }
  for (auto& shader : loadedShaders)
    glDeleteShader(shader);
}

ShaderProgram& ShaderProgram::use()
{
  glUseProgram(id);
  return *this;
}

GLuint ShaderProgram::name() const
{
  return id;
}

GLint ShaderProgram::getLocation(std::string_view name)
{
  GLint result = glGetUniformLocation(id, name.data());
  if (result < 0)
    log::println("Couldn't load name {} from program with id {}", name, id);
  return result;
}

