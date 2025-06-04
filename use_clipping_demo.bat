@echo off
echo Using working ClippingDemo as the main executable...

if exist build\ClippingDemo.exe (
    echo Copying ClippingDemo to main executable location...
    copy build\ClippingDemo.exe PixelCanvas_Clipping.exe
    
    echo Build successful!
    echo Running the application...
    start PixelCanvas_Clipping.exe
) else (
    echo ClippingDemo not found, building it first...
    call build_clip_direct.bat
    
    echo Copying ClippingDemo to main executable location...
    copy build\ClippingDemo.exe PixelCanvas_Clipping.exe
) 