cmake_minimum_required(VERSION 3.20)

project(GOLRenderer LANGUAGES CXX OBJCXX)

set(CMAKE_OBJCXX_STANDARD 23)

add_subdirectory(libs/logging)
add_subdirectory(libs/mathlib)
add_subdirectory(libs/file)

find_package(SDL3 REQUIRED)
find_library(METAL Metal REQUIRED)
find_library(FOUNDATION Foundation REQUIRED)
find_library(QUARTZ Quartz REQUIRED)

# compile commands for clangd
mark_as_advanced(CLEAR CMAKE_EXPORT_COMPILE_COMMANDS)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)

set(TargetApp GameOfLife)

set(APP_RESOURCES "Resources/MyShader.metallib")
set_source_files_properties(${APP_RESOURCES}
  PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

# files to compile for executable
add_executable(${TargetApp}
  src/main.mm
  ${APP_RESOURCES}
)

set(TargetSpecificLibs "")

if(WIN32)
  set(TargetSpecificLibs stdc++exp)
endif()

target_link_libraries(${TargetApp}
  PRIVATE
    SDL3::SDL3
    log
    Math
    ${TargetSpecificLibs}
    ${METAL}
    ${FOUNDATION}
    ${QUARTZ}
)

# include directory
target_include_directories(${TargetApp}
  PRIVATE
  ${CMAKE_SOURCE_DIR}/include
)


add_custom_target(CopyCompileCommands ALL
  COMMAND
    ${CMAKE_COMMAND} -E copy
      "${CMAKE_BINARY_DIR}/compile_commands.json"
      "${CMAKE_SOURCE_DIR}/compile_commands.json"
    COMMENT "Copying compile_commands"
)

  set_target_properties(${TargetApp}
  PROPERTIES
    MACOSX_BUNDLE_NAME            "GameOfLife"
    MACOSX_BUNDLE_VERSION         "1.0"
    MACOSX_BUNDLE_GUI_IDENTIFIER  "com.mrsamsh.si"
    MACOSX_BUNDLE_ICON_FILE       "icon.icns"
    MACOSX_BUNDLE   "TRUE"
  )

add_custom_target(CompileMetalShaders
  COMMAND xcrun -sdk macosx metal -c ${CMAKE_SOURCE_DIR}/shaders/default.metal -o MyShader.ir
  COMMAND xcrun -sdk macosx metal -o ${CMAKE_SOURCE_DIR}/Resources/MyShader.metallib MyShader.ir
  COMMENT "Compiling Shaders"
  VERBATIM
)

add_dependencies(${TargetApp} CompileMetalShaders)


add_dependencies(${TargetName} CopyCompileCommands)
