cmake_minimum_required(VERSION 3.20)

project(FILEIO)

# files to compile for executable
add_library(fileio STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/File.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/File.hpp
)

# include directory
target_include_directories(fileio
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(fileio
  PRIVATE
    cxx_generalized_initializers
    cxx_relaxed_constexpr
    cxx_std_20
)

