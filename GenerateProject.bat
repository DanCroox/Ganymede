@echo off

ECHO Generate assimp library project files ...
CALL buildscripts\BuildAssimp.bat -GenerateProject
IF %ERRORLEVEL% NEQ 0 (
	ECHO Error while generating assimp library project files.
	ECHO.
	PAUSE
	EXIT /b %ERRORLEVEL%
)

ECHO Generate bullet3 library project files ...
CALL buildscripts\BuildBullet3.bat -GenerateProject
IF %ERRORLEVEL% NEQ 0 (
	ECHO Error while generating bullet3 library project files.
	ECHO.
	PAUSE
	EXIT /b %ERRORLEVEL%
)

ECHO Generate glfw library project files ...
CALL buildscripts\BuildGlfw.bat -GenerateProject
IF %ERRORLEVEL% NEQ 0 (
	ECHO Error while generating glfw library project files.
	ECHO.
	PAUSE
	EXIT /b %ERRORLEVEL%
)

ECHO Generate glm library project files ...
CALL buildscripts\BuildGLM.bat -GenerateProject
IF %ERRORLEVEL% NEQ 0 (
	ECHO Error while generating glm library project files.
	ECHO.
	PAUSE
	EXIT /b %ERRORLEVEL%
)

ECHO Generate glew library project files ...
CALL buildscripts\BuildGlew.bat -GenerateProject
IF %ERRORLEVEL% NEQ 0 (
	ECHO Error while generating glew library project files.
	ECHO.
	PAUSE
	EXIT /b %ERRORLEVEL%
)

ECHO Generate Ganymede project files ...
CALL vendor\bin\premake\premake5.exe vs2022
IF %ERRORLEVEL% NEQ 0 (
	ECHO Error while generating Ganymede project files.
	ECHO.
	PAUSE
	EXIT /b %ERRORLEVEL%
)

ECHO Projects successfully generated.

ECHO.
PAUSE
