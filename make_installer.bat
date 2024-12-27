@echo off

set PROJECT_DIR=%~dp0

set BUILD_DIR=%PROJECT_DIR%build
if not exist %BUILD_DIR% mkdir %BUILD_DIR%

cd %BUILD_DIR%
cmake -S %PROJECT_DIR% -B %BUILD_DIR% -DCMAKE_BUILD_TYPE=Release

REM Build the projec
cmake --build %BUILD_DIR% --config Release

cpack -G WIX -C Release

cd ..