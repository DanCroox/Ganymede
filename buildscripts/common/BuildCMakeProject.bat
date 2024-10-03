@echo off

:: Check if cmake present
WHERE cmake >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    ECHO CMake not found. Please install cmake first!
    EXIT /b 1
)

SET SOURCE_DIR=%1
SET BUILD_CONFIG=%2

SET BUILD_DIR=%SOURCE_DIR%\build

ECHO "Build started ..."

ECHO %BUILD_DIR%
:: Check if project-build directory exists. Create if not.
IF NOT EXIST "%BUILD_DIR%" (
    ECHO "CMake build directory not existent. Regenerate projects!
    EXIT /b 1
)

:: Switch into project-build directory.
PUSHD "%BUILD_DIR%"

:: Build binaries.
ECHO "Build started ..."
cmake --build . -j20 --config %BUILD_CONFIG%

POPD

:: Check if build succeeded
IF %ERRORLEVEL% NEQ 0 (
    ECHO Build failed.
    EXIT /b 1
)

ECHO Build succeeded!
EXIT /b 0