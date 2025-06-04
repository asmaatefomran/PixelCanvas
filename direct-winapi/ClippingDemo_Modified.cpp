#include <windows.h>
#include <windowsx.h>
#include <string>
#include <vector>
#include <cmath>
#include <commctrl.h> // Add this for status bar

// For compatibility with older versions
#ifndef SBARS_SIZEGRIP
#define SBARS_SIZEGRIP 0x0100
#endif

// Link with comctl32.lib
#pragma comment(lib, "comctl32.lib")

// Include necessary headers for clipping
#include "../include/Clipping2D.h"

// Define application states
enum AppState {
    DEFINING_CLIP_WINDOW,
    LINE_CLIPPING,
    POLYGON_CLIPPING
};

// Global variables
HWND g_hwnd;
HWND g_hStatusBar;
RECT g_canvasRect;
HDC g_hdcMem = NULL;
HBITMAP g_hbmMem = NULL;

// Clipping state
Graphics::ClipWindow g_clipWindow;
bool g_clipWindowDefined = false;
int g_clipWindowClickCount = 0;
AppState g_currentState = DEFINING_CLIP_WINDOW;

// Line clipping state
bool g_drawingLine = false;
Graphics::Point g_lineStart;
Graphics::Point g_lineEnd;

// Polygon clipping state
std::vector<Graphics::Point> g_polygonPoints;

// Function prototypes
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitializeDrawingBuffer(HWND hwnd);
void CleanupDrawingBuffer();
void DrawClipWindow(HDC hdc);
void UpdateStatusBar(const wchar_t* message);
void ClearCanvas(HDC hdc);

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Initialize Common Controls for status bar
    InitCommonControls(); // Use this simpler version instead
    
    // Register window class
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"ClippingDemoClass";
    
    if (!RegisterClassW(&wc)) {
        MessageBoxW(NULL, L"Window Registration Failed", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    
    // Adjust window size for desired client area
    RECT windowRect = {0, 0, 800, 600};
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
    
    // Create the window
    g_hwnd = CreateWindowW(
        L"ClippingDemoClass",
        L"PixelCanvas - Sutherland-Hodgman Clipping",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
        NULL, NULL, hInstance, NULL
    );
    
    if (!g_hwnd) {
        MessageBoxW(NULL, L"Window Creation Failed", L"Error", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    
    // Create status bar
    g_hStatusBar = CreateWindowW(
        STATUSCLASSNAMEW, NULL,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0,
        g_hwnd, NULL, hInstance, NULL
    );
    
    // Set status bar text
    UpdateStatusBar(L"Right-click twice to define the clipping window, press C to clear, L for line clipping, P for polygon clipping");
    
    // Initialize drawing buffer
    InitializeDrawingBuffer(g_hwnd);
    
    // Show and update the window
    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);
    
    // Message loop
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // Clean up
    CleanupDrawingBuffer();
    
    return (int)msg.wParam;
}

// Status bar update function
void UpdateStatusBar(const wchar_t* message) {
    if (g_hStatusBar) {
        SendMessage(g_hStatusBar, SB_SETTEXT, 0, (LPARAM)message);
    }
}

// Initialize double-buffering
void InitializeDrawingBuffer(HWND hwnd) {
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    
    // Adjust client rect to exclude status bar
    RECT statusRect;
    GetWindowRect(g_hStatusBar, &statusRect);
    clientRect.bottom -= (statusRect.bottom - statusRect.top);
    
    g_canvasRect = clientRect;
    
    // Create memory device context
    HDC hdc = GetDC(hwnd);
    g_hdcMem = CreateCompatibleDC(hdc);
    g_hbmMem = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
    SelectObject(g_hdcMem, g_hbmMem);
    
    // Fill with white background
    RECT fillRect = {0, 0, clientRect.right, clientRect.bottom};
    FillRect(g_hdcMem, &fillRect, (HBRUSH)GetStockObject(WHITE_BRUSH));
    
    ReleaseDC(hwnd, hdc);
}

// Clean up drawing buffer
void CleanupDrawingBuffer() {
    if (g_hbmMem) {
        DeleteObject(g_hbmMem);
        g_hbmMem = NULL;
    }
    
    if (g_hdcMem) {
        DeleteDC(g_hdcMem);
        g_hdcMem = NULL;
    }
}

// Clear the canvas
void ClearCanvas(HDC hdc) {
    RECT fillRect = {0, 0, g_canvasRect.right, g_canvasRect.bottom};
    FillRect(hdc, &fillRect, (HBRUSH)GetStockObject(WHITE_BRUSH));
}

// Draw the clipping window
void DrawClipWindow(HDC hdc) {
    if (g_clipWindowDefined) {
        Graphics::Clipping2D clipper(hdc);
        clipper.drawClipWindow(RGB(0, 0, 255));
    }
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
            
        case WM_SIZE:
            {
                // Resize status bar
                if (g_hStatusBar) {
                    SendMessage(g_hStatusBar, WM_SIZE, 0, 0);
                }
                
                // Recreate the drawing buffer
                CleanupDrawingBuffer();
                InitializeDrawingBuffer(hwnd);
            }
            return 0;
            
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                
                // Blit the memory DC to the window DC
                BitBlt(hdc, g_canvasRect.left, g_canvasRect.top, 
                       g_canvasRect.right - g_canvasRect.left, 
                       g_canvasRect.bottom - g_canvasRect.top,
                       g_hdcMem, 0, 0, SRCCOPY);
                
                // If we're drawing a polygon, draw the current points and lines
                if (g_currentState == POLYGON_CLIPPING && !g_polygonPoints.empty()) {
                    // Draw points
                    for (const auto& p : g_polygonPoints) {
                        Ellipse(hdc, static_cast<int>(p.x) - 3, static_cast<int>(p.y) - 3, 
                               static_cast<int>(p.x) + 3, static_cast<int>(p.y) + 3);
                    }
                    
                    // Draw lines connecting points
                    if (g_polygonPoints.size() > 1) {
                        for (size_t i = 1; i < g_polygonPoints.size(); i++) {
                            MoveToEx(hdc, static_cast<int>(g_polygonPoints[i-1].x), 
                                   static_cast<int>(g_polygonPoints[i-1].y), NULL);
                            LineTo(hdc, static_cast<int>(g_polygonPoints[i].x), 
                                  static_cast<int>(g_polygonPoints[i].y));
                        }
                    }
                }
                
                EndPaint(hwnd, &ps);
            }
            return 0;
            
        case WM_RBUTTONDOWN:
            {
                int x = GET_X_LPARAM(lParam);
                int y = GET_Y_LPARAM(lParam);
                
                // Handle right-click for defining the clip window
                if (g_currentState == DEFINING_CLIP_WINDOW || !g_clipWindowDefined) {
                    if (g_clipWindowClickCount == 0) {
                        // First click - store first corner
                        g_clipWindow.left = x;
                        g_clipWindow.top = y;
                        g_clipWindowClickCount++;
                        
                        UpdateStatusBar(L"Right-click again to define the second corner of the clipping window");
                    } else {
                        // Second click - define the clip window
                        g_clipWindow.right = x;
                        g_clipWindow.bottom = y;
                        
                        // Normalize the window (ensure left < right and top < bottom)
                        if (g_clipWindow.right < g_clipWindow.left) {
                            double temp = g_clipWindow.left;
                            g_clipWindow.left = g_clipWindow.right;
                            g_clipWindow.right = temp;
                        }
                        
                        if (g_clipWindow.bottom < g_clipWindow.top) {
                            double temp = g_clipWindow.top;
                            g_clipWindow.top = g_clipWindow.bottom;
                            g_clipWindow.bottom = temp;
                        }
                        
                        g_clipWindowDefined = true;
                        g_clipWindowClickCount = 0;
                        g_currentState = LINE_CLIPPING;
                        
                        // Draw the clip window
                        DrawClipWindow(g_hdcMem);
                        
                        UpdateStatusBar(L"Clipping window defined. Left-click to start a line, press 'P' to switch to polygon clipping");
                        
                        // Force redraw
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                } else if (g_currentState == POLYGON_CLIPPING && g_polygonPoints.size() >= 3) {
                    // Complete the polygon and clip it
                    Graphics::Clipping2D clipper(g_hdcMem);
                    clipper.setClipWindow(g_clipWindow);
                    
                    std::vector<Graphics::Point> clippedPoints = clipper.clipPolygon(g_polygonPoints);
                    
                    if (!clippedPoints.empty()) {
                        // Draw the clipped polygon
                        HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
                        HPEN hOldPen = (HPEN)SelectObject(g_hdcMem, hPen);
                        HBRUSH hOldBrush = (HBRUSH)SelectObject(g_hdcMem, GetStockObject(NULL_BRUSH));
                        
                        // Create array of POINTs for Polygon function
                        POINT* polyPoints = new POINT[clippedPoints.size()];
                        for (size_t i = 0; i < clippedPoints.size(); i++) {
                            polyPoints[i].x = static_cast<LONG>(clippedPoints[i].x);
                            polyPoints[i].y = static_cast<LONG>(clippedPoints[i].y);
                        }
                        
                        Polygon(g_hdcMem, polyPoints, static_cast<int>(clippedPoints.size()));
                        
                        delete[] polyPoints;
                        SelectObject(g_hdcMem, hOldPen);
                        SelectObject(g_hdcMem, hOldBrush);
                        DeleteObject(hPen);
                    }
                    
                    // Reset polygon points
                    g_polygonPoints.clear();
                    
                    UpdateStatusBar(L"Polygon clipped. Left-click to start a new polygon or press 'L' to switch to line clipping");
                    
                    // Force redraw
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }
            return 0;
            
        case WM_LBUTTONDOWN:
            {
                int x = GET_X_LPARAM(lParam);
                int y = GET_Y_LPARAM(lParam);
                
                if (g_clipWindowDefined) {
                    if (g_currentState == LINE_CLIPPING) {
                        if (!g_drawingLine) {
                            // First point of the line
                            g_lineStart.x = x;
                            g_lineStart.y = y;
                            g_drawingLine = true;
                            
                            UpdateStatusBar(L"Left-click to set the end point of the line");
                        } else {
                            // Second point of the line
                            g_lineEnd.x = x;
                            g_lineEnd.y = y;
                            g_drawingLine = false;
                            
                            // Clip and draw the line
                            Graphics::Clipping2D clipper(g_hdcMem);
                            clipper.setClipWindow(g_clipWindow);
                            
                            Graphics::Point start = g_lineStart;
                            Graphics::Point end = g_lineEnd;
                            
                            if (clipper.clipLine(start, end)) {
                                // Draw the clipped line
                                HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
                                HPEN hOldPen = (HPEN)SelectObject(g_hdcMem, hPen);
                                
                                MoveToEx(g_hdcMem, static_cast<int>(start.x), static_cast<int>(start.y), NULL);
                                LineTo(g_hdcMem, static_cast<int>(end.x), static_cast<int>(end.y));
                                
                                SelectObject(g_hdcMem, hOldPen);
                                DeleteObject(hPen);
                            }
                            
                            // Also draw the original line in a different color
                            HPEN hPen = CreatePen(PS_DOT, 1, RGB(0, 128, 0));
                            HPEN hOldPen = (HPEN)SelectObject(g_hdcMem, hPen);
                            MoveToEx(g_hdcMem, static_cast<int>(g_lineStart.x), static_cast<int>(g_lineStart.y), NULL);
                            LineTo(g_hdcMem, static_cast<int>(g_lineEnd.x), static_cast<int>(g_lineEnd.y));
                            SelectObject(g_hdcMem, hOldPen);
                            DeleteObject(hPen);
                            
                            UpdateStatusBar(L"Line clipped. Left-click to start a new line or press 'P' to switch to polygon clipping");
                            
                            // Force redraw
                            InvalidateRect(hwnd, NULL, TRUE);
                        }
                    } else if (g_currentState == POLYGON_CLIPPING) {
                        // Add point to polygon
                        g_polygonPoints.push_back(Graphics::Point(x, y));
                        
                        // Draw a small circle at this point
                        Ellipse(g_hdcMem, x - 3, y - 3, x + 3, y + 3);
                        
                        // If there are at least 2 points, draw a line
                        if (g_polygonPoints.size() >= 2) {
                            int lastIndex = g_polygonPoints.size() - 1;
                            MoveToEx(g_hdcMem, static_cast<int>(g_polygonPoints[lastIndex - 1].x), 
                                   static_cast<int>(g_polygonPoints[lastIndex - 1].y), NULL);
                            LineTo(g_hdcMem, static_cast<int>(g_polygonPoints[lastIndex].x), 
                                  static_cast<int>(g_polygonPoints[lastIndex].y));
                        }
                        
                        UpdateStatusBar(L"Adding point to polygon. Right-click to complete and clip");
                        
                        // Force redraw
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                }
            }
            return 0;
            
        case WM_KEYDOWN:
            {
                if (wParam == 'C' || wParam == 'c') {
                    // Clear canvas and reset
                    ClearCanvas(g_hdcMem);
                    g_clipWindowDefined = false;
                    g_clipWindowClickCount = 0;
                    g_currentState = DEFINING_CLIP_WINDOW;
                    g_drawingLine = false;
                    g_polygonPoints.clear();
                    
                    UpdateStatusBar(L"Canvas cleared. Right-click twice to define clipping window");
                    
                    // Force redraw
                    InvalidateRect(hwnd, NULL, TRUE);
                } else if (wParam == 'L' || wParam == 'l') {
                    // Switch to line clipping mode
                    g_currentState = LINE_CLIPPING;
                    g_drawingLine = false;
                    g_polygonPoints.clear();
                    
                    UpdateStatusBar(L"Line clipping mode. Left-click to start a line");
                } else if (wParam == 'P' || wParam == 'p') {
                    // Switch to polygon clipping mode
                    g_currentState = POLYGON_CLIPPING;
                    g_drawingLine = false;
                    g_polygonPoints.clear();
                    
                    UpdateStatusBar(L"Polygon clipping mode. Left-click to add points, right-click to complete");
                }
            }
            return 0;
            
        case WM_ERASEBKGND:
            // To avoid flicker, don't erase the background
            return 1;
    }
    
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
} 