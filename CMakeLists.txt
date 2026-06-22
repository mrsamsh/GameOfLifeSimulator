cmake_minimum_required(VERSION 3.20)

project(GOLRenderer)

add_subdirectory(libs/logging)
add_subdirectory(libs/mathlib)
add_subdirectory(libs/file)

find_package(SDL3 REQUIRED)

# compile commands for clangd
mark_as_advanced(CLEAR CMAKE_EXPORT_COMPILE_COMMANDS)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)

set(TargetApp GameOfLife)

include(CompileShaders.cmake)

set_shader_type(SPIRV)

add_shader_library(SHADER Shader
  shaders/default_vert.hlsl
  shaders/default_frag.hlsl
)



set(GENERATED_HEADERS
  ${CMAKE_SOURCE_DIR}/shaders/default_vert.hpp
  ${CMAKE_SOURCE_DIR}/shaders/default_frag.hpp
)

if(WIN32)
  add_custom_command(
    OUTPUT ${GENERATED_HEADERS}
    COMMAND cmd /c "${CMAKE_SOURCE_DIR}/shaders/compile_dx_shaders.bat"
    DEPENDS
      ${CMAKE_SOURCE_DIR}/shaders/default_vert.hlsl
      ${CMAKE_SOURCE_DIR}/shaders/default_frag.hlsl
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/shaders
    COMMENT "Compiling DX shaders"
  )
elseif(LINUX)
  add_custom_command(
    OUTPUT ${GENERATED_HEADERS}
    COMMAND bash "${CMAKE_SOURCE_DIR}/shaders/compile_spv_shaders.sh"
    DEPENDS
      ${CMAKE_SOURCE_DIR}/shaders/default_vert.hlsl
      ${CMAKE_SOURCE_DIR}/shaders/default_frag.hlsl
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/shaders
    COMMENT "Compiling SPIR-V shaders"
  )
elseif(APPLE)
  add_custom_command(
    OUTPUT ${GENERATED_HEADERS}
    COMMAND bash "${CMAKE_SOURCE_DIR}/shaders/compile_metal_shaders.sh"
    DEPENDS
      ${CMAKE_SOURCE_DIR}/shaders/default_vert.metal
      ${CMAKE_SOURCE_DIR}/shaders/default_frag.metal
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/shaders
    COMMENT "Compiling Metal shaders"
  )
endif()

# files to compile for executable
add_executable(${TargetApp}
  src/main.cpp
  ${GENERATED_HEADERS}
  ${SHADER}
)

# set(TargetSpecificLibs "")

if(WIN32)
  set(TargetSpecificLibs stdc++exp)
endif()

target_link_libraries(${TargetApp}
  PRIVATE
    SDL3::SDL3
    log
    Math
    ${TargetSpecificLibs}
)

# include directory
target_include_directories(${TargetApp}
  PRIVATE
  ${CMAKE_SOURCE_DIR}/include
)

target_compile_features(${TargetApp}
  PRIVATE
    cxx_generalized_initializers
    cxx_relaxed_constexpr
    cxx_std_23
)

add_custom_target(CopyCompileCommands ALL
  COMMAND
    ${CMAKE_COMMAND} -E copy
      "${CMAKE_BINARY_DIR}/compile_commands.json"
      "${CMAKE_SOURCE_DIR}/compile_commands.json"
    COMMENT "Copying compile_commands"
)

if(APPLE)
set_target_properties(${TargetApp}
PROPERTIES
  MACOSX_BUNDLE_NAME            "GameOfLife"
  MACOSX_BUNDLE_VERSION         "1.0"
  MACOSX_BUNDLE_GUI_IDENTIFIER  "com.mrsamsh.si"
  MACOSX_BUNDLE_ICON_FILE       "icon.icns"
  MACOSX_BUNDLE   "TRUE"
)
endif()

target_compile_definitions(${TargetApp}
  PUBLIC $<$<CONFIG:Debug>:ASSERTION_ENABLED;DEBUG>
)

add_dependencies(${TargetApp} CopyCompileCommands)
