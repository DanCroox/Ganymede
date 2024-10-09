

set WORKING_DIR=%~dp0%

set IDE="Visual Studio 17 2022"
set ARCHITECTURE=x64
set SOURCE_DIR=%WORKING_DIR%..\Ganymede\vendor\bullet3
set BUILD_CONFIG=%~1

setlocal EnableDelayedExpansion
IF "%BUILD_CONFIG%"=="-GenerateProject" (
	set BUILD_PARAMS="-DUSE_MSVC_RUNTIME_LIBRARY_DLL=ON -DBUILD_UNIT_TESTS=OFF -DBUILD_EXTRAS=OFF -DBUILD_BULLET2_DEMOS=OFF -DBUILD_OPENGL3_DEMOS=OFF -DBUILD_ENET=OFF -DBUILD_CLSOCKET=OFF -DUSE_GLUT=OFF -DBUILD_CPU_DEMOS=OFF"
	%WORKING_DIR%common\GenerateCMakeProject.bat %IDE% %ARCHITECTURE% %SOURCE_DIR% !BUILD_PARAMS!
) else (
	%WORKING_DIR%common\BuildCMakeProject.bat %SOURCE_DIR% %BUILD_CONFIG%
)