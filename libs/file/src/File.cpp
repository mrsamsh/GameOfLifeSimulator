//
// File.cpp
// OpenGLBP
//
// Created by Usama Alshughry 13.07.2024.
// Copyright © 2024 Usama Alshughry. All rights reserved.
//

#include "File.hpp"

#include <cstdio>

namespace bp
{

std::optional<std::string> loadFile(std::string_view filename)
{
  FILE* file = fopen(filename.data(), "rb");
  if(file)
  {
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char buffer[size + 1];
    fread(buffer, sizeof(char), size, file);
    buffer[size] = '\0';
    return std::string(buffer);
  }
  return std::nullopt;
}


} // namespace bp
