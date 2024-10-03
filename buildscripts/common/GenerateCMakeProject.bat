@echo off

:: Check if cmake present
WHERE cmake >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    ECHO CMake not found. Please install cmake first!
    EXIT /b 1
)

SET IDE=%1
SET ARCHITECTURE=%2
SET SOURCE_DIR=%3
SET BUILD_PARAMS=%~4

SET BUILD_DIR=%SOURCE_DIR%\build

ECHO "Project generation started ..."

ECHO %BUILD_DIR%
:: Check if project-build directory exists. Create if not.
IF NOT EXIST "%BUILD_DIR%" (
    MKDIR "%BUILD_DIR%"
)

:: Switch into project-build directory.
PUSHD "%BUILD_DIR%"

:: Generate project files.
cmake %SOURCE_DIR% -G %IDE% -A %ARCHITECTURE% %BUILD_PARAMS%

POPD

:: Check if generation succeeded.
IF %ERRORLEVEL% NEQ 0 (
    ECHO CMake-config failed.
    EXIT /b 1
)

ECHO Project generation succeeded!
EXIT /b 0