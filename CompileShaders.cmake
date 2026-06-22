function(set_shader_type type)
  if("${type}" STREQUAL "METAL" OR "${type}" STREQUAL "SPIRV" OR "${type}" STREQUAL "DXIL")
    set(SHADER_TYPE ${type} PARENT_SCOPE)
  else()
    message(FATAL_ERROR "Unrecognized shader type ${type}. Supported types: METAL | SPIRV | DXIL")
  endif()
endfunction()

function(add_metal_shader_library libID libName)
  set(airFiles)
  set(libName "${libName}.metallib")
  set(SHADERS_DIR ${CMAKE_BINARY_DIR}/Shaders/Metal)
  foreach(file ${ARGN})
    cmake_path(GET file STEM airFilename)
    set(airFilename "${airFilename}.air")
    add_custom_command(
      OUTPUT ${SHADERS_DIR}/${airFilename}
      DEPENDS ${CMAKE_SOURCE_DIR}/${file}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
      COMMAND xcrun -sdk macosx metal -c ${CMAKE_SOURCE_DIR}/${file} -o ${SHADERS_DIR}/${airFilename}
      COMMENT "Compiling ${airFilename}"
      VERBATIM
    )
    list(APPEND airFiles "${SHADERS_DIR}/${airFilename}")
  endforeach()
  add_custom_command(
    OUTPUT ${SHADERS_DIR}/${libName}
    DEPENDS "${airFiles}"
    COMMAND xcrun -sdk macosx metal -o ${SHADERS_DIR}/${libName} ${airFiles}
    COMMENT "Linking Metal library ${libName}"
    VERBATIM
  )
  set("${libID}" ${SHADERS_DIR}/${libName} PARENT_SCOPE)
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
