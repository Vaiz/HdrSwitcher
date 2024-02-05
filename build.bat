@echo off
setlocal

REM Change to your project's root directory, if necessary
cd /d %~dp0

REM Configure the project using the x64-release preset
cmake --fresh -S . -B out/build/x64-release --preset x64-release

REM Build the project
cmake --build out/build/x64-release

echo Build completed.
endlocal
