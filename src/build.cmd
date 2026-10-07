@echo off
setlocal
cd /d "%~dp0"
rem MSYS2 MinGW x64 compiler and windres must be on PATH.
rem Recreate source.zip from player.c, README.txt, VERIFICATION.txt, app.ico,
rem app.manifest, include/vlc headers, app.rc and this build.cmd before compiling app.rc.
windres app.rc app-res.o
if errorlevel 1 exit /b 1
gcc -Iinclude -O2 -Wall -Wextra -Wno-misleading-indentation -municode -mwindows -static-libgcc player.c app-res.o -lshell32 -luser32 -ladvapi32 -o "NDI PLAY.exe"
