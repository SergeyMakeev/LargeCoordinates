@echo off
rem Build and test script for LargeCoordinates project

echo LargeCoordinates Build and Test Script
echo ======================================

rem Create build directory if it doesn't exist
if not exist build mkdir build

rem Change to build directory
cd build

rem Configure with CMake
echo Configuring project with CMake...
cmake ..
if errorlevel 1 (
    echo ERROR: CMake configuration failed
    pause
    exit /b 1
)

rem Build the project
echo Building project...
cmake --build .
if errorlevel 1 (
    echo ERROR: Build failed
    pause
    exit /b 1
)

rem Run tests directly
echo.
echo Running tests...
echo ================
Debug\test_large_coordinates.exe
if errorlevel 1 (
    echo ERROR: Tests failed
    pause
    exit /b 1
)

rem Run tests via CTest
echo.
echo Running CTest...
echo ================
ctest -C Debug --output-on-failure
if errorlevel 1 (
    echo ERROR: CTest failed
    pause
    exit /b 1
)

echo.
echo All tests passed successfully!
echo Build and test completed.
pause 