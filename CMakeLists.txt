cmake_minimum_required(VERSION 3.20)

project(GOLRenderer)

add_subdirectory(libs/glad)
add_subdirectory(libs/logging)
add_subdirectory(libs/mathlib)
add_subdirectory(libs/file)

find_package(SDL3 REQUIRED)

# compile commands for clangd
mark_as_advanced(CLEAR CMAKE_EXPORT_COMPILE_COMMANDS)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)

# files to compile for executable
add_executable(app
  src/ShaderProgram.cpp
  src/main.cpp
)

target_link_libraries(app
  PRIVATE
    Glad
    SDL3::SDL3
    log
    Math
    fileio
    fmt
)

# include directory
target_include_directories(app
  PRIVATE
  ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(app
  PRIVATE
    cxx_generalized_initializers
    cxx_relaxed_constexpr
    cxx_std_20
)

add_custom_command(
  TARGET app POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E create_symlink
      "${CMAKE_BINARY_DIR}/compile_commands.json"
      "${CMAKE_SOURCE_DIR}/compile_commands.json"
  # DEPENDS "${CMAKE_SOURCE_DIR}/compile_commands.json"
  COMMENT "Creating symlink for compile_commands"
)

