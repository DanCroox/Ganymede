@echo off

set WORKING_DIR=%~dp0%

set IDE="Visual Studio 17 2022"
set ARCHITECTURE=x64
set SOURCE_DIR=%WORKING_DIR%..\Ganymede\vendor\glfw
set BUILD_CONFIG=%~1

setlocal EnableDelayedExpansion
IF "%BUILD_CONFIG%"=="-GenerateProject" (
	set BUILD_PARAMS="-DUSE_MSVC_RUNTIME_LIBRARY_DLL=ON -DGLFW_BUILD_DOCS=OFF -DGLFW_INSTALL=OFF -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF"
	%WORKING_DIR%common\GenerateCMakeProject.bat %IDE% %ARCHITECTURE% %SOURCE_DIR% !BUILD_PARAMS!
) else (
	%WORKING_DIR%common\BuildCMakeProject.bat %SOURCE_DIR% %BUILD_CONFIG%
)

