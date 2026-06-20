@echo off
setlocal enabledelayedexpansion

set SM=6_0

echo Compiling HLSH shaders with DXC
echo ===============================

for %%f in (*.hlsl) do (
  set "filename=%%~nf"
  set "suffix=!filename:~-4!"
  
  if "!suffix!"=="vert" (
    set "target=vs_!SM!"
    set "entry=VSmain"
  ) else if "!suffix!"=="frag" (
    set "target=ps_!SM!"
    set "entry=FSmain"
  ) else (
    echo Unknown shader type for %%f, skipping.
    echo Name must be suffixed with vert or frag
    goto skip
  )

  echo Compiling %%f as !target!...
  dxc.exe -T !target! -Fo "!filename!.dxil" -E !entry! "%%f"
  xxd -n "!filename!" -i "!filename!.dxil" "!filename!.hpp"

  if %errorlevel% neq 0 (
    echo [ERROR] Failed to compile %%f
  ) else (
    echo [SUCCESS] Generated %%~nf.dxil
  )

  :skip
  echo -----------------------------
)
