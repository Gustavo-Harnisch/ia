@echo off
setlocal

rem Build in the Windows temporary directory, then remove generated build files.
set "PROYECTO_DIR=%~dp0"
set "BUILD_DIR=%TEMP%\steiner-build-%RANDOM%-%RANDOM%"

where cmake >nul 2>nul
if errorlevel 1 (
    echo Error: CMake is required. Install CMake and a C++17 compiler.
    exit /b 1
)
where dot >nul 2>nul
if errorlevel 1 (
    echo Error: Graphviz is required. Install Graphviz and add its bin folder to PATH.
    exit /b 1
)

echo Configuring project...
cmake -S "%PROYECTO_DIR%" -B "%BUILD_DIR%" -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 goto build_error

echo Compiling project...
cmake --build "%BUILD_DIR%" --config Release
if errorlevel 1 goto build_error

set "PROGRAM=%BUILD_DIR%\proyecto.exe"
if not exist "%PROGRAM%" set "PROGRAM=%BUILD_DIR%\Release\proyecto.exe"
if not exist "%PROGRAM%" (
    echo Error: the build completed but proyecto.exe was not found.
    goto build_error
)

echo Running project...
"%PROGRAM%" %*
set "RESULT=%ERRORLEVEL%"
call :cleanup
exit /b %RESULT%

:build_error
set "RESULT=1"
call :cleanup
exit /b %RESULT%

:cleanup
if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
exit /b 0
