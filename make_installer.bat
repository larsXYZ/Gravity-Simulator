@echo off
REM Set the project directory
set PROJECT_DIR=%~dp0

REM Configure the build directory
set BUILD_DIR=%PROJECT_DIR%build
if not exist %BUILD_DIR% mkdir %BUILD_DIR%

REM Run CMake to generate build files
cd %BUILD_DIR%
cmake -S %PROJECT_DIR% -B %BUILD_DIR% -DCMAKE_BUILD_TYPE=Release

REM Build the project
cmake --build %BUILD_DIR% --config Release

REM Generate the installer using CPack
cpack --config %BUILD_DIR%\CPackConfig.cmake

REM Check if the installer was created successfully
for /r %BUILD_DIR% %%f in (*.exe) do (
    if exist "%%f" (
        echo Installer created successfully: %%f
        goto :eof
    )
)
echo Failed to create installer.
