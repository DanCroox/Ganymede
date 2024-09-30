@echo off

call buildscripts\BuildAssimp.bat -GenerateProject
call buildscripts\BuildBullet3.bat -GenerateProject
call buildscripts\BuildGlfw.bat -GenerateProject
call buildscripts\BuildGLM.bat -GenerateProject

call vendor\bin\premake\premake5.exe vs2022

ECHO.
PAUSE