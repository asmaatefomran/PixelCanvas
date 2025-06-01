@echo off
echo Building PixelCanvas...
g++ -std=c++17 -O2 direct-winapi/PixelCanvas_fixed.cpp src/Circle.cpp src/Line.cpp -Iinclude -lgdi32 -lcomctl32 -lcomdlg32 -mwindows -o PixelCanvas.exe
if %ERRORLEVEL% NEQ 0 (
    echo Error: Compilation failed.
    pause
    exit /b 1
)
echo Compilation successful!
echo Running PixelCanvas...
start PixelCanvas.exe
exit /b 0 