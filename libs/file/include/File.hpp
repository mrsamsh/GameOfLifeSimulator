//
// File.hpp
// OpenGLBP
//
// Created by Usama Alshughry 13.07.2024.
// Copyright © 2024 Usama Alshughry. All rights reserved.
//

#ifndef FILE_HPP_
#define FILE_HPP_

#include <optional>
#include <string_view>
#include <string>

namespace bp
{

std::optional<std::string> loadFile(std::string_view filename);

} // namespace bp

#endif // FILE_HPP_
