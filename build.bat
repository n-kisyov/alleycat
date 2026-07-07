@echo off
setlocal enabledelayedexpansion

echo ============================================
echo         Alley Cat - Build Script
echo ============================================
echo.

set "BUILD_DIR=build"
set "CONFIG=Release"

where cmake >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo [ERROR] CMake not found. Install from https://cmake.org/download/
    pause
    exit /b 1
)

echo [1/2] Configuring and generating...
cmake -B %BUILD_DIR% -S . -Wno-dev
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Configuration failed.
    pause
    exit /b %ERRORLEVEL%
)

echo [2/2] Building %CONFIG%...
cmake --build %BUILD_DIR% --config %CONFIG%
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Build failed.
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo ============================================
echo   Build succeeded.
echo   %CD%\%BUILD_DIR%\%CONFIG%\alleycat.exe
echo ============================================
echo.
set /p RUN="Run now? [Y/N]: "
if /i "!RUN!"=="Y" (
    echo Launching Alley Cat...
    start "" "%BUILD_DIR%\%CONFIG%\alleycat.exe"
)

endlocal
