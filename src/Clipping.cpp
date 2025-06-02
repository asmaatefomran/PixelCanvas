#include <windows.h>
#include "../include/LineClipping.h"
#include "../include/PolygonClipping.h"

// Global variables for clipping
bool clippingMode = false;
bool lineClippingMode = false;
bool polygonClippingMode = false;
bool definingClipWindow = false;
bool drawingPolygon = false;

// Reset all clipping states
void resetClippingStates() {
    clippingMode = false;
    lineClippingMode = false;
    polygonClippingMode = false;
    definingClipWindow = false;
    drawingPolygon = false;
    cnt = 0;
    cnt2 = 0;
    pointCount = 0;
    polygonDrawn = false;
}

// Initialize clipping window with default values
void initClippingWindow() {
    xleft = defaultXLeft;
    ytop = defaultYTop;
    xright = defaultXRight;
    ybottom = defaultYBottom;
}

// Handle line clipping input
bool handleLineClippingInput(HWND hwnd, int x, int y, bool isRightClick) {
    if (!lineClippingMode) return false;

    if (isRightClick) {
        // Define clipping window with right click
        if (cnt == 0) {
            in[0].x = x;
            in[0].y = y;
            cnt++;
            return true;
        } 
        else if (cnt == 1) {
            in[1].x = x;
            in[1].y = y;
            cnt++;
            getSquareWindow();
            drawWindowBorder(hwnd);
            definingClipWindow = true;
            return true;
        }
    } 
    else if (definingClipWindow) {
        // Define line with left click after window is defined
        if (cnt2 < polygonSize) {
            polygon[cnt2].x = x;
            polygon[cnt2].y = y;
            cnt2++;
            
            if (cnt2 == polygonSize) {
                HDC hdc = GetDC(hwnd);
                CohenSuth(hdc, polygon[0].x, polygon[0].y, polygon[1].x, polygon[1].y);
                ReleaseDC(hwnd, hdc);
                cnt2 = 0;
            }
            return true;
        }
    }

    return false;
}

// Handle polygon clipping input
bool handlePolygonClippingInput(HWND hwnd, int x, int y, bool isRightClick) {
    if (!polygonClippingMode) return false;

    if (!isRightClick && !polygonDrawn && pointCount < 100) {
        // Add point to polygon with left click
        polygon[pointCount].x = x;
        polygon[pointCount].y = y;
        pointCount++;
        
        // Draw connecting lines as we add points
        if (pointCount > 1) {
            HDC hdc = GetDC(hwnd);
            DrawLine(hdc, polygon[pointCount-2].x, polygon[pointCount-2].y, 
                    polygon[pointCount-1].x, polygon[pointCount-1].y, RGB(255, 255, 255));
            ReleaseDC(hwnd, hdc);
        }
        drawingPolygon = true;
        return true;
    } 
    else if (isRightClick && drawingPolygon && pointCount > 2) {
        // Complete polygon with right click
        HDC hdc = GetDC(hwnd);
        
        // Draw closing edge
        DrawLine(hdc, polygon[pointCount-1].x, polygon[pointCount-1].y, 
                polygon[0].x, polygon[0].y, RGB(255, 255, 255));
        
        // Draw clipping window
        DrawLine(hdc, xleft, ytop, xright, ytop, RGB(255, 0, 0));      // Top edge
        DrawLine(hdc, xright, ytop, xright, ybottom, RGB(255, 0, 0));  // Right edge
        DrawLine(hdc, xright, ybottom, xleft, ybottom, RGB(255, 0, 0)); // Bottom edge
        DrawLine(hdc, xleft, ybottom, xleft, ytop, RGB(255, 0, 0));    // Left edge
        
        // Apply clipping
        PolygonClip(hdc, polygon, pointCount, xleft, ytop, xright, ybottom);
        
        ReleaseDC(hwnd, hdc);
        polygonDrawn = true;
        return true;
    }

    return false;
}

// Draw the current clipping window
void drawClippingWindow(HWND hwnd) {
    HDC hdc = GetDC(hwnd);
    
    // Draw clipping window with red color
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
    HGDIOBJ oldPen = SelectObject(hdc, hPen);

    MoveToEx(hdc, xleft, ytop, NULL);
    LineTo(hdc, xright, ytop);
    LineTo(hdc, xright, ybottom);
    LineTo(hdc, xleft, ybottom);
    LineTo(hdc, xleft, ytop);

    SelectObject(hdc, oldPen);
    DeleteObject(hPen);
    ReleaseDC(hwnd, hdc);
}

// Start line clipping mode
void startLineClippingMode() {
    resetClippingStates();
    lineClippingMode = true;
    clippingMode = true;
    initClippingWindow();
}

// Start polygon clipping mode
void startPolygonClippingMode() {
    resetClippingStates();
    polygonClippingMode = true;
    clippingMode = true;
    initClippingWindow();
    xleft = defaultXLeft;
    ytop = defaultYTop;
    xright = defaultXRight;
    ybottom = defaultYBottom;
} 