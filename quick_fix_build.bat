@echo off
echo Building simplified working solution...

:: Compile ClippingDemo_Modified which is already working
g++ direct-winapi\ClippingDemo_Modified.cpp ^
    src\Clipping2D.cpp ^
    src\Line2D.cpp ^
    -o WorkingClippingDemo.exe ^
    -lgdi32 -lcomctl32 -lcomdlg32

if %ERRORLEVEL% EQU 0 (
    echo Build successful!
    echo Running application...
    start WorkingClippingDemo.exe
) else (
    echo Build failed with error level %ERRORLEVEL%
    pause
) 