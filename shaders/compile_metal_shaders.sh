#!/bin/bash

echo "Compiling metal shaders to metallib."

for file in *.metal; do
  filename=$(basename ${file%.*})
  filetype=${filename: -4}
  if [[ "${filetype}" == "vert" ]]; then
    entry="VSmain"
  else
    entry="FSmain"
  fi
  echo "Compiling $file..."
  xcrun -sdk macosx metal -c ${file} -o ${filename}.air
  xcrun -sdk macosx metal -o ${filename}.metallib ${filename}.air
  xxd -n "${filename}" -i "${filename}.metallib" "${filename}.hpp"
done
