#include "../include/Clipping2D.h"
#include "../include/LineClipping.h" // Include this to get the Point type
#include "../include/PolygonClipping.h" // Include for PolygonClip function
#include <windows.h>
#include <stdio.h>
#include <algorithm>

extern RECT g_canvasRect;
extern int g_lineThickness;

// External variables from LineClipping.h
extern Point in[2];
extern const int polygonSize;
extern Point polygon[polygonSize];
extern int cnt, cnt2;
extern double xleft, xright, ytop, ybottom;

// External variables from PolygonClipping.h
extern POINT polyPoints[100]; // Renamed from polygon to avoid name conflict
extern int polyPointCount;    // Renamed from pointCount

// Global variables for our implementation
bool g_clipWindowDefined = false;
Graphics::ClipWindow g_clipWindow(0, 0, 0, 0);
COLORREF g_clipWindowColor = RGB(255, 0, 0);  // Red for clip window
std::vector<Graphics::Point> g_clipLinePoints;
std::vector<Graphics::Point> g_clipPolygonPoints;

// Forward declarations
void drawWindowBorder(HWND hwnd);
void UpdateStatusBar(const wchar_t* message);
void DrawLine(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color, int thickness);
void CohenSuth(HDC hdc, double xs, double ys, double xe, double ye);

// Implement handleLineClippingInput function
bool handleLineClippingInput(HWND hwnd, int x, int y, bool isRightClick) {
    if (isRightClick) {
        // Handle right-click for defining the clipping window
        if (!g_clipWindowDefined) {
            // First right-click: store the first corner of the window
            if (cnt == 0) {
                in[0].x = x;
                in[0].y = y;
                cnt++;
                
                // Draw a point to indicate the first corner
                HDC hdc = GetDC(hwnd);
                SetPixel(hdc, x + g_canvasRect.left, y + g_canvasRect.top, RGB(255, 0, 0));
                ReleaseDC(hwnd, hdc);
                
                UpdateStatusBar(L"Right-click again to define the opposite corner of the clipping window");
                return true;
            }
            // Second right-click: store the second corner and define the clipping window
            else if (cnt == 1) {
                in[1].x = x;
                in[1].y = y;
                cnt++;
                
                // Define the clipping window
                xleft = std::min(in[0].x, in[1].x);
                xright = std::max(in[0].x, in[1].x);
                ytop = std::min(in[0].y, in[1].y); 
                ybottom = std::max(in[0].y, in[1].y);
                
                // Set the clipping window for our new class
                g_clipWindow.left = xleft;
                g_clipWindow.top = ytop;
                g_clipWindow.right = xright;
                g_clipWindow.bottom = ybottom;
                g_clipWindowDefined = true;
                
                // Draw the clipping window
                drawWindowBorder(hwnd);
                
                // Update status bar
                UpdateStatusBar(L"Clipping window defined. Left-click to set the start point of a line");
                
                return true;
            }
        }
        
        return false;
    }
    
    // Handle left-click for line points
    if (g_clipWindowDefined) {
        // We're collecting line endpoints
        if (cnt2 < polygonSize) {
            polygon[cnt2].x = x;
            polygon[cnt2].y = y;
            cnt2++;
            
            // Draw a small point to mark the position
            HDC hdc = GetDC(hwnd);
            SetPixel(hdc, x + g_canvasRect.left, y + g_canvasRect.top, RGB(0, 255, 0));
            ReleaseDC(hwnd, hdc);
            
            if (cnt2 == 1) {
                UpdateStatusBar(L"Left-click to set the end point of the line");
            }
        }
        
        // If we have two points, perform clipping
        if (cnt2 == polygonSize) {
            HDC hdc = GetDC(hwnd);
            
            // Draw original line with blue color
            DrawLine(hdc, polygon[0].x, polygon[0].y, polygon[1].x, polygon[1].y, RGB(0, 0, 255), 1);
            
            // Use Cohen-Sutherland from LineClipping.h for compatibility with existing code
            CohenSuth(hdc, polygon[0].x, polygon[0].y, polygon[1].x, polygon[1].y);
            
            // Also draw using our new Clipping2D class with green color
            Graphics::Clipping2D clipper(hdc);
            clipper.setClipWindow(g_clipWindow);
            
            Graphics::Point start(polygon[0].x, polygon[0].y);
            Graphics::Point end(polygon[1].x, polygon[1].y);
            
            // Draw the clip window again
            clipper.drawClipWindow(g_clipWindowColor);
            
            // Clip and draw the line if visible
            if (clipper.clipLine(start, end)) {
                // Draw clipped line with green color and increased thickness
                HPEN hPen = CreatePen(PS_SOLID, g_lineThickness + 1, RGB(0, 255, 0));
                HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
                
                MoveToEx(hdc, (int)start.x, (int)start.y, NULL);
                LineTo(hdc, (int)end.x, (int)end.y);
                
                SelectObject(hdc, hOldPen);
                DeleteObject(hPen);
            }
            
            ReleaseDC(hwnd, hdc);
            
            // Reset for the next line
            cnt2 = 0;
            
            // Update status bar
            UpdateStatusBar(L"Line clipped. Left-click to start a new line or right-click to define a new clipping window");
            
            return true;
        }
    }
    
    return false;
}

// Implement handlePolygonClippingInput function
bool handlePolygonClippingInput(HWND hwnd, int x, int y, bool isRightClick) {
    if (isRightClick) {
        // Handle right-click for defining the clipping window or completing a polygon
        if (!g_clipWindowDefined) {
            // Same as line clipping for window definition
            if (cnt == 0) {
                in[0].x = x;
                in[0].y = y;
                cnt++;
                
                // Draw a point to indicate the first corner
                HDC hdc = GetDC(hwnd);
                SetPixel(hdc, x + g_canvasRect.left, y + g_canvasRect.top, RGB(255, 0, 0));
                ReleaseDC(hwnd, hdc);
                
                UpdateStatusBar(L"Right-click again to define the opposite corner of the clipping window");
                return true;
            }
            else if (cnt == 1) {
                in[1].x = x;
                in[1].y = y;
                cnt++;
                
                // Define the clipping window
                xleft = std::min(in[0].x, in[1].x);
                xright = std::max(in[0].x, in[1].x);
                ytop = std::min(in[0].y, in[1].y);
                ybottom = std::max(in[0].y, in[1].y);
                
                // Set the clipping window for our new class
                g_clipWindow.left = xleft;
                g_clipWindow.top = ytop;
                g_clipWindow.right = xright;
                g_clipWindow.bottom = ybottom;
                g_clipWindowDefined = true;
                
                // Draw the clipping window
                drawWindowBorder(hwnd);
                
                // Update status bar
                UpdateStatusBar(L"Clipping window defined. Left-click to add polygon vertices, right-click to complete polygon");
                
                return true;
            }
        }
        else if (polyPointCount >= 3) {
            // We have enough points to clip a polygon
            HDC hdc = GetDC(hwnd);
            
            // Draw original polygon
            HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
            HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
            
            // Draw the original polygon
            for (int i = 0; i < polyPointCount - 1; i++) {
                MoveToEx(hdc, polyPoints[i].x, polyPoints[i].y, NULL);
                LineTo(hdc, polyPoints[i + 1].x, polyPoints[i + 1].y);
            }
            // Close the polygon
            MoveToEx(hdc, polyPoints[polyPointCount - 1].x, polyPoints[polyPointCount - 1].y, NULL);
            LineTo(hdc, polyPoints[0].x, polyPoints[0].y);
            
            SelectObject(hdc, hOldPen);
            DeleteObject(hPen);
            
            // Use Sutherland-Hodgman algorithm from PolygonClipping.h
            PolygonClip(hdc, polyPoints, polyPointCount, xleft, ytop, xright, ybottom);
            
            // Also use our new Clipping2D class
            Graphics::Clipping2D clipper(hdc);
            clipper.setClipWindow(g_clipWindow);
            
            // Convert points to Graphics::Point vector
            std::vector<Graphics::Point> polygonPoints;
            for (int i = 0; i < polyPointCount; i++) {
                polygonPoints.push_back(Graphics::Point(polyPoints[i].x, polyPoints[i].y));
            }
            
            // Clip the polygon
            std::vector<Graphics::Point> clippedPoints = clipper.clipPolygon(polygonPoints);
            
            if (!clippedPoints.empty()) {
                // Draw the clipped polygon with green color
                hPen = CreatePen(PS_SOLID, g_lineThickness + 1, RGB(0, 255, 0));
                hOldPen = (HPEN)SelectObject(hdc, hPen);
                
                // Draw the clipped polygon
                for (size_t i = 0; i < clippedPoints.size() - 1; i++) {
                    MoveToEx(hdc, (int)clippedPoints[i].x, (int)clippedPoints[i].y, NULL);
                    LineTo(hdc, (int)clippedPoints[i + 1].x, (int)clippedPoints[i + 1].y);
                }
                // Close the polygon
                if (clippedPoints.size() > 1) {
                    MoveToEx(hdc, (int)clippedPoints[clippedPoints.size() - 1].x, (int)clippedPoints[clippedPoints.size() - 1].y, NULL);
                    LineTo(hdc, (int)clippedPoints[0].x, (int)clippedPoints[0].y);
                }
                
                SelectObject(hdc, hOldPen);
                DeleteObject(hPen);
            }
            
            // Redraw the clipping window to ensure it's visible
            clipper.drawClipWindow(g_clipWindowColor);
            
            ReleaseDC(hwnd, hdc);
            
            // Reset polygon points
            polyPointCount = 0;
            
            // Update status bar
            UpdateStatusBar(L"Polygon clipped. Left-click to begin a new polygon");
            
            return true;
        }
        
        return false;
    }
    
    // Handle left-click for polygon vertices
    if (g_clipWindowDefined) {
        // Add point to the polygon
        if (polyPointCount < 100) {  // Limit to avoid overflow
            polyPoints[polyPointCount].x = x;
            polyPoints[polyPointCount].y = y;
            polyPointCount++;
            
            // Draw a small point to mark the vertex
            HDC hdc = GetDC(hwnd);
            SetPixel(hdc, x + g_canvasRect.left, y + g_canvasRect.top, RGB(0, 0, 255));
            ReleaseDC(hwnd, hdc);
            
            // If we have at least one point, draw lines connecting them
            if (polyPointCount > 1) {
                HDC hdc = GetDC(hwnd);
                
                // Draw line from previous point to current point
                MoveToEx(hdc, polyPoints[polyPointCount - 2].x, polyPoints[polyPointCount - 2].y, NULL);
                LineTo(hdc, polyPoints[polyPointCount - 1].x, polyPoints[polyPointCount - 1].y);
                
                ReleaseDC(hwnd, hdc);
            }
            
                        // Update status
            wchar_t statusMsg[256];
            _snwprintf(statusMsg, 256, L"Added point %d. Left-click to add more points, right-click to complete polygon",
                polyPointCount);
            UpdateStatusBar(statusMsg);
            
            return true;
        }
    }
    
    return false;
} 