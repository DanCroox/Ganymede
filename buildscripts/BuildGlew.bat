@echo off

set WORKING_DIR=%~dp0%

set IDE="Visual Studio 17 2022"
set ARCHITECTURE=x64
set SOURCE_DIR=%WORKING_DIR%..\Ganymede\vendor\glew\build\cmake
set BUILD_CONFIG=%~1

setlocal EnableDelayedExpansion
IF "%BUILD_CONFIG%"=="-GenerateProject" (
	set BUILD_PARAMS="-DCMAKE_POLICY_VERSION_MINIMUM=3.5"
	%WORKING_DIR%common\GenerateCMakeProject.bat %IDE% %ARCHITECTURE% %SOURCE_DIR% !BUILD_PARAMS!
) else (
	%WORKING_DIR%common\BuildCMakeProject.bat %SOURCE_DIR% %BUILD_CONFIG%
)