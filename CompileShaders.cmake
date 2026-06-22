set(SHADER_TYPE "METAL" CACHE STRING "Select shader type")
set_property(CACHE SHADER_TYPE PROPERTY STRINGS "METAL" "SPIRV" "DXIL")

function(set_shader_type type)
  if("${type}" STREQUAL "METAL" OR "${type}" STREQUAL "SPIRV" OR "${type}" STREQUAL "DXIL")
    set(SHADER_TYPE ${type} PARENT_SCOPE)
  else()
    message(FATAL_ERROR "Unrecognized shader type ${type}. Supported types: METAL | SPIRV | DXIL")
  endif()
endfunction()

function(add_metal_shader_library libID libName)
  set(SHADERS_DIR ${CMAKE_BINARY_DIR}/Shaders/Metal)
  foreach(file ${ARGN})
    cmake_path(GET file STEM filename)
    string(LENGTH "${filename}" filenameLength)
    math(EXPR start_index "${filenameLength} - 4")
    string(SUBSTRING "${filename}" ${start_index} 4 shaderType)
    set(airFilename "${SHADERS_DIR}/${filename}.air")
    set(targetShader ${SHADERS_DIR}/${libName}.${shaderType}.metallib)
    add_custom_command(
      OUTPUT ${targetShader}
      DEPENDS ${CMAKE_SOURCE_DIR}/${file}
      COMMAND xcrun -sdk macosx metal -c ${CMAKE_SOURCE_DIR}/${file} -o ${airFilename}
      COMMAND xcrun -sdk macosx metal -o ${targetShader} ${airFilename}
      COMMAND ${CMAKE_COMMAND} -E rm ${airFilename}
      COMMENT "Compiling ${shaderType} shader"
      VERBATIM
    )
    list(APPEND compiledShaders "${targetShader}")
  endforeach()
  set("${libID}" ${compiledShaders} PARENT_SCOPE)
endfunction()

function(add_dxil_shader_library libID libName)
  set(SHADERS_DIR ${CMAKE_BINARY_DIR}/Shaders/DXIL)
  foreach(file ${ARGN})
    cmake_path(GET file STEM filename)
    string(LENGTH "${filename}" filenameLength)
    math(EXPR start_index "${filenameLength} - 4")
    string(SUBSTRING "${filename}" ${start_index} 4 shaderType)
    set(targetShader ${SHADERS_DIR}/${libName}.${shaderType}.dxil)
    if("${shaderType}" STREQUAL "vert")
      add_custom_command(
        OUTPUT ${targetShader}
        DEPENDS ${CMAKE_SOURCE_DIR}/${file}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
        COMMAND dxc.exe -T vs_6_0 -Fo ${targetShader} -E VSmain ${CMAKE_SOURCE_DIR}/${file}
        COMMENT "Compiling ${shaderType} shader"
        VERBATIM
      )
    elseif("${shaderType}" STREQUAL "frag")
      add_custom_command(
        OUTPUT ${targetShader}
        DEPENDS ${CMAKE_SOURCE_DIR}/${file}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
        COMMAND dxc.exe -T ps_6_0 -Fo ${targetShader} -E FSmain ${CMAKE_SOURCE_DIR}/${file}
        COMMENT "Compiling ${shaderType} shader"
        VERBATIM
      )
    else()
      message(FATAL_ERROR "Unknown shader type in ${file}, naming convention: name should end with vert or frag")
    endif()
    list(APPEND compiledShaders ${targetShader})
  endforeach()
  set("${libID}" ${compiledShaders} PARENT_SCOPE)
endfunction()

function(add_spv_shader_library libID libName)
  set(SHADERS_DIR ${CMAKE_BINARY_DIR}/Shaders/SPIRV)
  foreach(file ${ARGN})
    cmake_path(GET file STEM filename)
    string(LENGTH "${filename}" filenameLength)
    math(EXPR start_index "${filenameLength} - 4")
    string(SUBSTRING "${filename}" ${start_index} 4 shaderType)
    set(targetShader ${SHADERS_DIR}/${libName}.${shaderType}.spv)
    if("${shaderType}" STREQUAL "vert")
      add_custom_command(
        OUTPUT ${targetShader}
        DEPENDS ${CMAKE_SOURCE_DIR}/${file}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
        COMMAND glslc -DUSING_GLSLC=1 -fshader-stage=${shaderType} -fentry-point=VSmain -c ${CMAKE_SOURCE_DIR}/${file} -o ${targetShader}
        COMMENT "Compiling ${shaderType} shader"
        VERBATIM
      )
    elseif("${shaderType}" STREQUAL "frag")
      add_custom_command(
        OUTPUT ${targetShader}
        DEPENDS ${CMAKE_SOURCE_DIR}/${file}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
        COMMAND glslc -DUSING_GLSLC=1 -fshader-stage=${shaderType} -fentry-point=FSmain -c ${CMAKE_SOURCE_DIR}/${file} -o ${targetShader}
        COMMENT "Compiling ${shaderType} shader"
        VERBATIM
      )
    else()
      message(FATAL_ERROR "Unknown shader type in ${file}, naming convention: name should end with vert or frag")
    endif()
    list(APPEND compiledShaders ${targetShader})
  endforeach()
  set("${libID}" ${compiledShaders} PARENT_SCOPE)
endfunction()

macro(add_shader_library libID libName)
  if("${SHADER_TYPE}" STREQUAL "METAL")
    add_metal_shader_library(${libID} ${libName} ${ARGN})
  elseif("${SHADER_TYPE}" STREQUAL "SPIRV")
    add_spv_shader_library(${libID} ${libName} ${ARGN})
  elseif("${SHADER_TYPE}" STREQUAL "DXIL")
    add_dxil_shader_library(${libID} ${libName} ${ARGN})
  else()
    message(FATAL_ERROR "Shader type is not specified. Set by calling (set_shader_type).")
  endif()
endmacro()

function(generate_header_files genName genDir)
  foreach(file ${ARGN})
    cmake_path(GET file FILENAME filename)
    cmake_path(REMOVE_EXTENSION filename LAST_ONLY)
    set(targetHeader ${genDir}/${filename}.hpp)
    add_custom_command(
      OUTPUT ${targetHeader}
      DEPENDS ${file}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${genDir}
      COMMAND xxd -n ${filename} -i ${file} ${targetHeader}
      COMMENT "Generating header for ${filename}"
      VERBATIM
    )
    list(APPEND generatedHeaders "${targetHeader}")
  endforeach()
  set("${genName}" ${generatedHeaders} PARENT_SCOPE)
endfunction()
