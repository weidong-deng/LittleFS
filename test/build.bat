@echo off
REM LittleFS Test Build Script

REM Enter script directory
cd /d "%~dp0"

REM Check tools
echo Checking build environment...
gcc --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: GCC not found
    pause
    exit /b 1
)

cmake --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: CMake not found
    pause
    exit /b 1
)

echo GCC and CMake OK

REM Remove old build directory
if exist build (
    echo Removing old build directory...
    rmdir /s /q build
)

REM Create build directory
echo Creating build directory...
mkdir build
if errorlevel 1 (
    echo ERROR: Cannot create build directory
    pause
    exit /b 1
)

REM Run CMake
cd build
echo Running CMake configure...
cmake .. -G "MinGW Makefiles"
if errorlevel 1 (
    echo ERROR: CMake configure failed
    cd ..
    pause
    exit /b 1
)

REM Build
echo Building...
mingw32-make -j8
if errorlevel 1 (
    echo ERROR: Build failed
    cd ..
    pause
    exit /b 1
)

echo.
echo ========================================
echo Build SUCCESS
echo ========================================
echo.

REM Run tests
set /p run_tests="Run tests? (Y/N): "
if /i "%run_tests%"=="Y" (
    echo Running tests...
    ctest --output-on-failure | findstr /V /C:"Start "

    echo.
    echo Generating coverage report...
    if not exist coverage mkdir coverage
    gcovr --root ..\.. --exclude ".*Unity.*" --html --html-details -o coverage\coverage_report.html .
    if errorlevel 1 (
        echo Coverage tool not available, skipping coverage report
    ) else (
        echo Coverage report generated: coverage\coverage_report.html
    )
)

cd ..
pause
