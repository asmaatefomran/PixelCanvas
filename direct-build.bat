@echo off
echo Building PixelCanvas direct implementation...

g++ direct-winapi/PixelCanvas_fixed.cpp src/Line.cpp src/Circle.cpp -o PixelCanvas_direct.exe -lgdi32 -luser32 -lshell32 -lcomctl32 -Iinclude -static-libgcc -static-libstdc++ -mwindows

if %ERRORLEVEL% NEQ 0 (
    echo Error: Compilation failed.
    pause
) else (
    echo Compilation successful.
    echo Running PixelCanvas...
    PixelCanvas_direct.exe
) 