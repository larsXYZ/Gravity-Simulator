@echo off
setlocal

:menu
echo ===========================================
echo Select the architecture to build:
echo ===========================================
echo 1. x86
echo 2. x64
echo ===========================================
set /p choice="Enter your choice (1 or 2): "

if "%choice%"=="1" (
    set ARCH=Win32
    goto setup
) else if "%choice%"=="2" (
    set ARCH=x64
    goto setup
) else (
    echo Invalid choice, please select 1 or 2.
    goto menu
)

:setup
set PROJECT_DIR=%~dp0
set BUILD_DIR=%PROJECT_DIR%build_%ARCH%
if not exist %BUILD_DIR% mkdir %BUILD_DIR%

cd %BUILD_DIR%
cmake -S %PROJECT_DIR% -B %BUILD_DIR% -DCMAKE_BUILD_TYPE=Release -A %ARCH%

REM Build the project
cmake --build %BUILD_DIR% --config Release

cpack -G WIX -C Release

cd %PROJECT_DIR%
endlocal
