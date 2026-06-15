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

# files to compile for executable
add_executable(${TargetApp}
  src/main.cpp
)

target_link_libraries(${TargetApp}
  PRIVATE
    SDL3::SDL3
    log
    Math
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

if (WIN32)
  add_custom_target(CompileDXShaders ALL
    COMMENT "Compiling DX12 shaders"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/shaders
    COMMAND cmd /c "${CMAKE_SOURCE_DIR}/shaders/compile_shaders.bat"
  )
  add_dependencies(${TargetName} CompileDXShaders)
endif()


add_dependencies(${TargetName} CopyCompileCommands)
