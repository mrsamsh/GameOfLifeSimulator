cmake_minimum_required(VERSION 3.20)

project(MathLib)

# compile commands for clangd
mark_as_advanced(CLEAR CMAKE_EXPORT_COMPILE_COMMANDS)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)

# files to compile for executable
add_library(Math INTERFACE)

# include directory
target_include_directories(Math
  INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(Math
  INTERFACE
    cxx_generalized_initializers
    cxx_relaxed_constexpr
    cxx_std_23
)

add_executable(testMath EXCLUDE_FROM_ALL
  src/main.cpp)

target_include_directories(testMath
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  )

target_compile_features(testMath
  PRIVATE
    cxx_generalized_initializers
    cxx_relaxed_constexpr
    cxx_std_23
)

add_custom_command(
  TARGET testMath POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E create_symlink
      "${CMAKE_BINARY_DIR}/compile_commands.json"
      "${CMAKE_SOURCE_DIR}/compile_commands.json"
  COMMENT "Creating symlink for compile_commands"
)

enable_testing()

add_test(
  NAME TestVector
  COMMAND $<TARGET_FILE:testMath>
)
