@echo off

set WORKING_DIR=%~dp0%

set IDE="Visual Studio 17 2022"
set ARCHITECTURE=x64
set SOURCE_DIR=%WORKING_DIR%..\Ganymede\vendor\assimp
set BUILD_CONFIG=%~1

setlocal EnableDelayedExpansion
IF "%BUILD_CONFIG%"=="-GenerateProject" (
	set BUILD_PARAMS="-DCMAKE_BUILD_TYPE=%BUILD_CONFIG% -DBUILD_SHARED_LIBS=OFF -DUSE_STATIC_CRT=ON"
	%WORKING_DIR%common\GenerateCMakeProject.bat %IDE% %ARCHITECTURE% %SOURCE_DIR% !BUILD_PARAMS!
) else (
	%WORKING_DIR%common\BuildCMakeProject.bat %SOURCE_DIR% %BUILD_CONFIG%
)