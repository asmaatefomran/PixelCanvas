@echo off
echo Building simplified PixelCanvas with minimal filling functionality...

g++ direct-winapi/PixelCanvas_fixed.cpp src/Line.cpp src/Circle.cpp -DSIMPLE_FILL -o PixelCanvas_simplified.exe -lgdi32 -luser32 -lshell32 -lcomctl32 -Iinclude -static-libgcc -static-libstdc++ -mwindows

if %ERRORLEVEL% NEQ 0 (
    echo Error: Compilation failed.
    pause
) else (
    echo Compilation successful.
    echo Running simplified PixelCanvas...
    PixelCanvas_simplified.exe
) 