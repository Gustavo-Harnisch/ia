@echo off
setlocal

rem Build a persistent Windows executable in the project directory.
set "PROYECTO_DIR=%~dp0"
set "BUILD_DIR=%TEMP%\steiner-build-%RANDOM%-%RANDOM%"
set "EXE=%PROYECTO_DIR%proyecto.exe"

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

set "BUILT_EXE=%BUILD_DIR%\proyecto.exe"
if not exist "%BUILT_EXE%" set "BUILT_EXE=%BUILD_DIR%\Release\proyecto.exe"
if not exist "%BUILT_EXE%" (
    echo Error: the build completed but proyecto.exe was not found.
    goto build_error
)

copy /y "%BUILT_EXE%" "%EXE%" >nul
if errorlevel 1 goto build_error
echo Windows executable created: "%EXE%"
call :cleanup
exit /b 0

:build_error
call :cleanup
exit /b 1

:cleanup
if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
exit /b 0
