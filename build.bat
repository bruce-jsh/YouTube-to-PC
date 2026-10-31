@echo off
rem Builds YouTube to PC and its installer.
rem The installer carries only the program itself; yt-dlp, FFmpeg and QuickJS are
rem downloaded while installing, so nothing else is needed to build.
setlocal EnableExtensions
cd /d "%~dp0"

rem ---- configuration -------------------------------------------------------
set "VERSION=1.3.0"
rem Set MSYS_BIN beforehand to build elsewhere, which is what CI does.
if not defined MSYS_BIN set "MSYS_BIN=C:\msys64\ucrt64\bin"
rem --------------------------------------------------------------------------

if not exist "%MSYS_BIN%\g++.exe" (
    echo [ERROR] g++ was not found in %MSYS_BIN%
    exit /b 1
)
set "PATH=%MSYS_BIN%;%PATH%"
set "CXXFLAGS=-std=c++17 -O2 -municode -Wall -Wno-unused-parameter -Wno-missing-field-initializers -Wno-cast-function-type"
set "LIBS=-lcomctl32 -lgdiplus -lshlwapi -luxtheme -ldwmapi -lurlmon -lwinhttp -lole32 -loleaut32 -luuid -lshell32 -lcomdlg32 -lgdi32 -luser32 -ladvapi32"
set "APP_SRC=app/main.cpp app/ui.cpp app/util.cpp app/model.cpp app/dialogs.cpp app/lang.cpp"

if not exist build\obj mkdir build\obj
if not exist dist mkdir dist

echo [1/5] YouTubeToPC.exe
windres -i app/app.rc -O coff -o build/obj/app.res.o || goto :fail
g++ %CXXFLAGS% -mwindows %APP_SRC% build/obj/app.res.o -o build/YouTubeToPC.exe -static -s %LIBS% || goto :fail

echo [2/5] installer stub
windres -i setup/setup.rc -O coff -o build/obj/setup.res.o || goto :fail
g++ %CXXFLAGS% -mwindows -Iapp setup/setup.cpp app/ui.cpp app/util.cpp app/lang.cpp build/obj/setup.res.o -o build/setup-stub.exe -static -s %LIBS% -lcabinet -lwininet || goto :fail

echo [3/5] packer
g++ %CXXFLAGS% tools/pack.cpp -o build/pack.exe -static -s -lcabinet || goto :fail

echo [4/5] payload
if exist build\payload rmdir /s /q build\payload
if exist build\payload (
    echo [ERROR] build\payload could not be cleared. Close any running YouTubeToPC and retry.
    goto :fail
)
mkdir build\payload\licenses
copy /y build\YouTubeToPC.exe build\payload\ >nul || goto :fail
copy /y licenses\THIRD-PARTY-NOTICES.txt build\payload\licenses\ >nul || goto :fail

echo [5/6] installer
build\pack.exe build\setup-stub.exe build\payload "dist\YouTubeToPC-Setup-%VERSION%.exe" || goto :fail

rem Browsers refuse downloads of unsigned programs they have not seen before,
rem so the release carries the same installer inside a zip as a way through.
echo [6/6] zipped copy
if exist "dist\YouTubeToPC-Setup-%VERSION%.zip" del /q "dist\YouTubeToPC-Setup-%VERSION%.zip"
tar.exe -a -c -f "dist\YouTubeToPC-Setup-%VERSION%.zip" -C dist "YouTubeToPC-Setup-%VERSION%.exe" || goto :fail

for /f "skip=1 delims=" %%h in ('certutil -hashfile "dist\YouTubeToPC-Setup-%VERSION%.exe" SHA256') do if not defined SHA256 set "SHA256=%%h"

echo.
echo Done.
echo   Installer : dist\YouTubeToPC-Setup-%VERSION%.exe
echo   Zipped    : dist\YouTubeToPC-Setup-%VERSION%.zip
echo   SHA-256   : %SHA256%
echo.
echo Publish it with:
echo   gh release create v%VERSION% "dist\YouTubeToPC-Setup-%VERSION%.exe" "dist\YouTubeToPC-Setup-%VERSION%.zip" --title "YouTube to PC %VERSION%"
exit /b 0

:fail
echo.
echo [ERROR] Build failed.
exit /b 1
