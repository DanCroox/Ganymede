@echo off

set SOURCE_DIR=%1
set BUILD_CONFIG=%2

set BUILD_DIR=%SOURCE_DIR%\build

echo "Build started ..."

echo %BUILD_DIR%
:: Check if project-build directory exists. Create if not.
if not exist "%BUILD_DIR%" (
    echo "CMake build directory not existent. Regenerate projects!
    exit /b 1
)

:: Switch into project-build directory.
pushd "%BUILD_DIR%"

:: Build binaries.
echo "Build started ..."
cmake --build . -j20 --config %BUILD_CONFIG%

popd

:: Check if build succeeded
if errorlevel 1 (
    echo Build failed.
    exit /b 1
)

echo Build succeeded!
exit /b 0