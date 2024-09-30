@echo off

set IDE=%1
set ARCHITECTURE=%2
set SOURCE_DIR=%3
set BUILD_PARAMS=%~4

set BUILD_DIR=%SOURCE_DIR%\build

echo "Project generation started ..."

echo %BUILD_DIR%
:: Check if project-build directory exists. Create if not.
if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)

:: Switch into project-build directory.
pushd "%BUILD_DIR%"

:: Generate project files.
cmake %SOURCE_DIR% -G %IDE% -A %ARCHITECTURE% %BUILD_PARAMS%

popd

:: Check if generation succeeded.
if errorlevel 1 (
    echo CMake-config failed.
    exit /b 1
)

echo Project generation succeeded!
exit /b 0