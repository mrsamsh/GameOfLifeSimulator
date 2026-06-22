#!/bin/bash

echo "Compiling hlsl shaders to SPV."

for file in *.hlsl; do
  filename=$(basename ${file%.*})
  filetype=${filename: -4}
  if [[ "${filetype}" == "vert" ]]; then
    entry="VSmain"
  else
    entry="FSmain"
  fi
  echo "Compiling $file..."
  glslc -DUSING_GLSLC=1 -fshader-stage=$filetype -fentry-point=$entry -c $file -o "${filename}.spv"
  xxd -n "${filename}" -i "${filename}.spv" "${filename}.hpp"
done
