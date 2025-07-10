cmake_minimum_required(VERSION 3.20)

project(logging)

# files to compile for executable
add_library(log INTERFACE)

# include directory
target_include_directories(log
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

