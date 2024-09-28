@echo off

cmake -G "Visual Studio 17 2022" -S Ganymede/vendor/assimp -B Ganymede/vendor/assimp/build -DBUILD_SHARED_LIBS=OFF

call vendor\bin\premake\premake5.exe vs2022
ECHO.

PAUSE