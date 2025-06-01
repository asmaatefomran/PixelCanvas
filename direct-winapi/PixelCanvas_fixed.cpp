#define _WIN32_IE 0x0500
#define _WIN32_WINNT 0x0500
#define WINVER 0x0500
#define UNICODE
#define _UNICODE

#include <windows.h>
#include <shellapi.h>
#include <string>
#include <commctrl.h>
#include <vector>
#include <windowsx.h>
#include <commdlg.h>
#include <math.h>
#include <iostream>
#include <algorithm> // For std::max, std::min
#include <cmath>     // For std::abs
#include <queue>     // For queue in flood fill
#include <climits>   // For INT_MAX, INT_MIN

// Include custom circle class
#include "../include/Circle.h"
// Include custom line class
#include "../include/Line.h"

// Include the common controls library (Visual Studio only)
#if defined(_MSC_VER)
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")
#endif

// Forward declarations for DrawCircle dependencies
void FillCircle(HDC hdc, int centerX, int centerY, int radius, COLORREF color);
void StandardFill(HDC hdc, int centerX, int centerY, int radius, COLORREF color);

// Define common control constants if not already defined
#ifndef BTNS_BUTTON
#define BTNS_BUTTON TBSTYLE_BUTTON
#endif

#ifndef BTNS_AUTOSIZE
#define BTNS_AUTOSIZE TBSTYLE_AUTOSIZE
#endif

#ifndef TBSTYLE_FLAT
#define TBSTYLE_FLAT 0x0800
#endif

// Define menu IDs
#define ID_FILE_EXIT 1001
#define ID_FILE_OPEN 1002
#define ID_FILE_SAVE 1003
#define ID_FILE_CLEAR 1004

// Circle quarter fill options
#define ID_QUARTER_FULL 8001  // Fill the whole circle
#define ID_QUARTER_1 8002     // Top-right quarter
#define ID_QUARTER_2 8003     // Top-left quarter
#define ID_QUARTER_3 8004     // Bottom-left quarter
#define ID_QUARTER_4 8005     // Bottom-right quarter

// Drawing tools
#define ID_TOOL_LINE 2001
#define ID_TOOL_CIRCLE 2002
#define ID_TOOL_ELLIPSE 2003
#define ID_TOOL_CURVE 2004
#define ID_TOOL_POLYGON 2005
#define ID_TOOL_CLIP 2006
#define ID_TOOL_FILL 2007  // Add new fill tool

// Algorithms
#define ID_ALGO_DDA 3001
#define ID_ALGO_MIDPOINT 3002
#define ID_ALGO_PARAMETRIC 3003
#define ID_ALGO_POLAR 3004
#define ID_ALGO_BEZIER 3005
#define ID_ALGO_HERMITE 3006
#define ID_ALGO_COHEN_SUTHERLAND 3007
#define ID_ALGO_LIANG_BARSKY 3008
#define ID_ALGO_DIRECT 3009
#define ID_ALGO_CARDINAL 3010

// Fill methods
#define ID_FILL_LINES 4001
#define ID_FILL_CIRCLES 4002
#define ID_FILL_GENERAL_POLYGON 4003
#define ID_FILL_CONVEX_POLYGON 4004
#define ID_FILL_FLOOD 4005
#define ID_FILL_FLOOD_QUEUE 4006
#define ID_FILL_CHECKBOX 8001
// Quarter fill methods (lines)
#define ID_FILL_QUARTER1_LINES 4007
#define ID_FILL_QUARTER2_LINES 4008
#define ID_FILL_QUARTER3_LINES 4009
#define ID_FILL_QUARTER4_LINES 4010
// Quarter fill methods (circles)
#define ID_FILL_QUARTER1_CIRCLES 4011
#define ID_FILL_QUARTER2_CIRCLES 4012
#define ID_FILL_QUARTER3_CIRCLES 4013
#define ID_FILL_QUARTER4_CIRCLES 4014

// Colors
#define ID_COLOR_BLACK 5001
#define ID_COLOR_RED 5002
#define ID_COLOR_GREEN 5003
#define ID_COLOR_BLUE 5004
#define ID_COLOR_CUSTOM 5005

// Help
#define ID_HELP_ABOUT 6001

// Controls
#define ID_COMBO_SHAPE 7001
#define ID_COMBO_ALGORITHM 7002
#define ID_COLOR_BUTTON 7003
#define ID_THICKNESS_SLIDER 7004
#define ID_LABEL_SHAPE 7005
#define ID_LABEL_ALGORITHM 7006
#define ID_LABEL_THICKNESS 7007
#define ID_LABEL_COLOR 7008
#define ID_COMBO_TENSION 7009
#define ID_POLY_FILL_LABEL 9010
#define ID_POLY_FILL_COMBO 9011

// Window size
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800

// Drawing area
#define CANVAS_MARGIN 10
#define TOOLBAR_HEIGHT 40
#define CONTROLS_HEIGHT 110
#define STATUSBAR_HEIGHT 20

// Colors
const COLORREF COLOR_BLACK = RGB(0, 0, 0);
const COLORREF COLOR_RED = RGB(255, 0, 0);
const COLORREF COLOR_GREEN = RGB(0, 128, 0);
const COLORREF COLOR_BLUE = RGB(0, 0, 255);
const COLORREF COLOR_WHITE = RGB(255, 255, 255);
const COLORREF COLOR_LIGHTGRAY = RGB(230, 230, 230);
const COLORREF COLOR_DARKGRAY = RGB(80, 80, 80);

// Global variables
HWND g_hwnd;
HMENU g_hMenuBar;
HMENU g_hFileMenu;
HMENU g_hToolMenu;
HMENU g_hAlgoMenu;
HMENU g_hFillMenu;
HMENU g_hColorMenu;
HMENU g_hHelpMenu;
HWND g_hToolbar;
HWND g_hStatusBar;
HWND g_hShapeCombo;
HWND g_hAlgorithmCombo;
HWND g_hFillCheckbox;  // Use a simple checkbox instead of radio buttons
HWND g_hQuarterCombo; // New dropdown for quarter selection
HWND g_hQuarterLabel; // Label for quarter selection
HWND g_hPolyFillLabel; // Label for polygon fill options
HWND g_hPolyFillCombo; // Dropdown for polygon fill options
HWND g_hColorButton;
HWND g_hThicknessSlider;
HWND g_hShapeLabel;
HWND g_hAlgorithmLabel;
HWND g_hFillMethodLabel; // Add this new label
HWND g_hThicknessLabel;
HWND g_hColorLabel;
HWND g_hHelpText;
HWND g_hCardinalTensionCombo;
std::wstring g_htmlPath;
HBRUSH g_hBackgroundBrush;
HBRUSH g_hCanvasBrush;
HFONT g_hLabelFont;
HFONT g_hButtonFont;

// Drawing state
COLORREF g_currentColor = COLOR_BLACK;
int g_currentTool = ID_TOOL_LINE;
int g_currentAlgorithm = ID_ALGO_DDA;
bool g_isFilled = false;  // Initialize to false to respect user choice
int g_lineThickness = 1;
int g_selectedQuarter = ID_QUARTER_FULL; // Default to fill the whole circle
int g_polyFillMethod = 0; // 0 = convex, 1 = non-convex (general), 2 = flood fill
bool g_isDrawing = false;
POINT g_startPoint = {0, 0};
POINT g_endPoint = {0, 0};
std::vector<POINT> g_polygonPoints;

// For ellipse 3-point drawing
int g_ellipseClickCount = 0;
POINT g_ellipseCenter = {0, 0};
POINT g_ellipsePoint1 = {0, 0};
POINT g_ellipsePoint2 = {0, 0};
int g_ellipseA = 0; // Semi-major axis
int g_ellipseB = 0; // Semi-minor axis

// Double buffering
HDC g_hdcMem = NULL;
HBITMAP g_hbmMem = NULL;
RECT g_canvasRect = {0, 0, 0, 0};

// Add a global variable to track when we're in ellipse drawing mode
bool g_ellipseDrawingMode = false;

// Add global variable for Cardinal Spline tension
float g_cardinalTension = 0.5f; // Default tension value

// Define point struct for polygon filling
struct point {
    double x, y;
    point(double x = 0, double y = 0) : x(x), y(y) {}
};

// Move all polygon filling functions declarations to before other forward declarations
// Forward declare all filling functions
void DrawLineDDA(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c);
void fillGeneralPolygon(HDC hdc, point p[], int n, COLORREF c);
void convexfill(HDC hdc, point p[], int n, COLORREF c);
void myFloodFill(HDC hdc, int x, int y, COLORREF bc, COLORREF fc);
void myFloodFillQueue(HDC hdc, int x, int y, COLORREF bc, COLORREF fc);
bool IsPolygonConvex(const std::vector<point>& points);

// Add these structs and functions before the forward declarations
struct Point {
    double x, y;
};

struct Derivative {
    double u, v;
};

// Rename the edgeTable typedef to avoid conflict with the global edgeTable array
struct EdgeRec {
    int left, right;
};
// Increase table size from 800 to 3000 to handle larger vertical areas
typedef EdgeRec convexEdgeTable[3000];

// Initialize edge table with default values
void init(convexEdgeTable tbl) {
    for (int i = 0; i < 3000; i++) {
        tbl[i].left = INT_MAX;
        tbl[i].right = INT_MIN;
    }
}

// Add an edge to the edge table
void edge2table(convexEdgeTable tbl, point v1, point v2) {
    try {
        // Skip horizontal edges
        if (fabs(v1.y - v2.y) < 0.001)
            return;
        
        // Ensure v1 is the lower point
        if (v1.y > v2.y)
            std::swap(v1, v2);
        
        // Validate coordinates are within reasonable range
        if (v1.y < -1000 || v1.y > 10000 || v2.y < -1000 || v2.y > 10000 ||
            v1.x < -1000 || v1.x > 10000 || v2.x < -1000 || v2.x > 10000) {
            // Skip this edge if coordinates are extreme
            return;
        }
        
        // Calculate scan line intersection parameters
        double x = v1.x;
        double slope;
        
        // Check for vertical line to avoid division by zero
        if (fabs(v2.y - v1.y) < 0.001) {
            slope = 0;  // Avoid division by zero
        } else {
            slope = (v2.x - v1.x) / (v2.y - v1.y);
        }
        
        // Calculate the scan line range, ensuring we stay within bounds
        int startY = (int)ceil(v1.y);
        startY = std::max(0, startY);
        startY = std::min(startY, 2999);
        
        int endY = (int)floor(v2.y);
        endY = std::max(0, endY);
        endY = std::min(endY, 2999);
        
        // For each scan line intersecting this edge
        for (int y = startY; y < endY; y++) {
            // Update min/max x values for this scan line
            int xInt = (int)ceil(x);
            
            // Bound check the x coordinate
            xInt = std::max(-1000, xInt);
            xInt = std::min(5000, xInt);
            
            // Update the edge table safely
            if (y >= 0 && y < 3000) {
                tbl[y].left = std::min(tbl[y].left, xInt);
                tbl[y].right = std::max(tbl[y].right, xInt);
            }
            
            // Move to next scan line
            x += slope;
        }
    }
    catch (...) {
        // Silent catch to prevent crashes
    }
}

// Add all polygon edges to the edge table
void polygon2table(convexEdgeTable tbl, point p[], int n) {
    // Process each edge of the polygon
    for (int i = 0; i < n; i++) {
        int next = (i + 1) % n;
        edge2table(tbl, p[i], p[next]);
    }
}

// Draw horizontal spans between left and right edges
void table2screen(HDC hdc, convexEdgeTable tbl, COLORREF c) {
    try {
        // Set a reasonable limit for the maximum span width to prevent hangs
        const int MAX_SPAN_WIDTH = 2000;
        
        // For each scan line
        for (int y = 0; y < 3000; y++) {
            // If we have a valid span (left <= right)
            if (tbl[y].left <= tbl[y].right && 
                tbl[y].left < 10000 && tbl[y].right < 10000 && 
                tbl[y].left > -10000 && tbl[y].right > -10000) { // Avoid extreme values
                
                // Ensure the span isn't too wide (would cause hangs)
                int spanWidth = tbl[y].right - tbl[y].left;
                if (spanWidth > MAX_SPAN_WIDTH) {
                    tbl[y].right = tbl[y].left + MAX_SPAN_WIDTH;
                }
                
                // Draw the horizontal line, but only if Y is in a reasonable range
                if (y >= 0 && y < 2000) {
                    // Clamp x values to reasonable limits
                    int left = std::max(0, tbl[y].left);
                    int right = std::min(5000, tbl[y].right);
                    
                    if (left < right) {
                        DrawLineDDA(hdc, left, y, right, y, c);
                    }
                }
            }
        }
    }
    catch (...) {
        // Silent catch to prevent crashes
    }
}

// Convex polygon fill function using scan line algorithm
void convexfill(HDC hdc, point p[], int n, COLORREF c) {
    // Ensure we have a valid polygon
    if (n < 3) return;
    
    try {
        // Create and initialize the edge table
        convexEdgeTable tbl;
        init(tbl);
        
        // Build the edge table from polygon edges
        polygon2table(tbl, p, n);
        
        // Draw the filled polygon using the edge table
        table2screen(hdc, tbl, c);
    }
    catch (...) {
        // Silently catch any exceptions to prevent crashes
        // Output error to debug console
        OutputDebugStringW(L"Error filling convex polygon");
    }
}

// ... rest of the filling code implementations ...

// Add Round function definition before it's used
int Round(double x) {
    return (int)(x + 0.5);
}

// Implementation of DrawLineDDA
void DrawLineDDA(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c)
{
    try {
        // Use our Line class implementation
        Line line(hdc);
        line.DrawLineDDA(x1, y1, x2, y2, c);
    }
    catch (...) {
        // Fallback implementation if Line class fails
        int dx = x2 - x1;
        int dy = y2 - y1;
        int steps;

        if (abs(dx) > abs(dy))
            steps = abs(dx);
        else
            steps = abs(dy);
            
        if (steps == 0) {
            SetPixel(hdc, x1, y1, c);
            return;
        }

        double x_increment = (double)dx / steps,
            y_increment = (double)dy / steps,
            x = x1,
            y = y1;

        SetPixel(hdc, Round(x), Round(y), c);

        for (int i = 0; i < steps; i++) {
            x += x_increment;
            y += y_increment;
            SetPixel(hdc, Round(x), Round(y), c);
        }
    }
}

void DrawHermite(HDC hdc, Point p1, Point p2, Derivative t1, Derivative t2, COLORREF c, int thickness) {
    double hermiteMatrix[4][4] = {
        { 2, 1, -2, 1 },
        {-3, -2, 3, -1 },
        { 0, 1, 0, 0 },
        { 1, 0, 0, 0 }
    };

    double gx[4] = { p1.x, t1.u, p2.x, t2.u };
    double gy[4] = { p1.y, t1.v, p2.y, t2.v };

    double cx[4] = {}, cy[4] = {};

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            cx[i] += hermiteMatrix[i][j] * gx[j];
            cy[i] += hermiteMatrix[i][j] * gy[j];
        }
    }

    // Use more points for a smoother curve with thicker lines
    POINT prevPoint = { 0, 0 };
    bool firstPoint = true;
    HPEN hPen = CreatePen(PS_SOLID, thickness, c);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

    for (double t = 0; t <= 1; t += 0.01) {
        double xt = cx[0] * pow(t, 3) + cx[1] * pow(t, 2) + cx[2] * t + cx[3];
        double yt = cy[0] * pow(t, 3) + cy[1] * pow(t, 2) + cy[2] * t + cy[3];

        POINT currentPoint = { (LONG)round(xt), (LONG)round(yt) };
        
        if (!firstPoint) {
            MoveToEx(hdc, prevPoint.x, prevPoint.y, NULL);
            LineTo(hdc, currentPoint.x, currentPoint.y);
        }
        
        prevPoint = currentPoint;
        firstPoint = false;
    }
    
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

// Forward declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateMenus(HWND hwnd);
void CreateToolbar(HWND hwnd);
void CreateStatusBar(HWND hwnd);
void CreateControls(HWND hwnd);
void HandleMenuSelection(HWND hwnd, WPARAM wParam);
void UpdateStatusBar(const wchar_t* message);
void InitializeDrawingBuffer(HWND hwnd);
void CleanupDrawingBuffer();
void DrawLine(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color, int thickness);
void DrawCircle(HDC hdc, int centerX, int centerY, int radius, COLORREF color, int thickness, bool filled);
void DrawEllipse(HDC hdc, int centerX, int centerY, int radiusX, int radiusY, COLORREF color, int thickness, bool filled);
void ClearCanvas(HDC hdc);
void SaveCanvasToBitmap(HWND hwnd);
void ShowColorDialog(HWND hwnd);
void UpdateColorButton();
RECT GetCanvasRect(HWND hwnd);
void UpdateControlsFromSelection();
void HandleShapeSelection();
void HandleAlgorithmSelection();
void HandleFillMethodSelection();
void HandleQuarterSelection();
void UpdateControlVisibility();
void HandlePolyFillSelection();
HFONT CreateCustomFont(int size, bool bold);
void UpdateAlgorithmDropdown();
void UpdateInstructions();
void UpdateCardinalControls(HWND hwnd, bool isCardinalSelected);
void DrawCardinalSpline(HDC hdc, const std::vector<POINT>& points, COLORREF color, int thickness);

// Update status bar with ellipse drawing progress
void UpdateEllipseStatus() {
    // Call the main instructions update function which now handles
    // ellipse instructions based on the click count
    UpdateInstructions();
}

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Initialize common controls
    // First call InitCommonControls for compatibility
    InitCommonControls();
    
    // Get current directory
    wchar_t currentDir[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, currentDir);
    g_htmlPath = currentDir;
    g_htmlPath += L"\\index.html";

    // Create background brushes
    g_hBackgroundBrush = CreateSolidBrush(COLOR_LIGHTGRAY);
    g_hCanvasBrush = CreateSolidBrush(COLOR_WHITE);
    
    // Create fonts
    g_hLabelFont = CreateCustomFont(14, false);
    g_hButtonFont = CreateCustomFont(14, true);

    // Register the window class
    const wchar_t CLASS_NAME[] = L"PixelCanvasClass";

    WNDCLASSW wc = {}; // Note the W suffix for Unicode
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = g_hBackgroundBrush;
    wc.style = CS_HREDRAW | CS_VREDRAW;

    RegisterClassW(&wc);

    // Create the window
    g_hwnd = CreateWindowExW(
        0,                             // Optional window styles
        CLASS_NAME,                    // Window class
        L"PixelCanvas - Drawing Application",  // Window text
        WS_OVERLAPPEDWINDOW,           // Window style
        CW_USEDEFAULT, CW_USEDEFAULT,  // Position
        WINDOW_WIDTH, WINDOW_HEIGHT,   // Size
        NULL,                          // Parent window
        NULL,                          // Menu
        hInstance,                     // Instance handle
        NULL                           // Additional application data
    );

    if (g_hwnd == NULL) {
        MessageBoxW(NULL, L"Failed to create window", L"Error", MB_OK);
        return 0;
    }

    // Initialize global variables
    // Circle* g_circle = nullptr;

    // Create UI elements
    CreateMenus(g_hwnd);
    CreateStatusBar(g_hwnd);
    CreateControls(g_hwnd);

    // Show the window
    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);

    // Initialize drawing buffer
    InitializeDrawingBuffer(g_hwnd);

    // Update instructions in the status bar
    UpdateInstructions();

    // Update control visibility based on current tool
    UpdateControlVisibility();

    // Run the message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Clean up
    CleanupDrawingBuffer();
    DeleteObject(g_hBackgroundBrush);
    DeleteObject(g_hCanvasBrush);
    DeleteObject(g_hLabelFont);
    DeleteObject(g_hButtonFont);
    
    // Clean up the circle object if it exists
    // if (g_circle != nullptr) {
    //     delete g_circle;
    //     g_circle = nullptr;
    // }

    return 0;
}

void CreateMenus(HWND hwnd) {
    // Create menu bar
    g_hMenuBar = CreateMenu();

    // Create File menu
    g_hFileMenu = CreateMenu();
    AppendMenuW(g_hFileMenu, MF_STRING, ID_FILE_OPEN, L"&Open HTML");
    AppendMenuW(g_hFileMenu, MF_STRING, ID_FILE_SAVE, L"&Save...");
    AppendMenuW(g_hFileMenu, MF_STRING, ID_FILE_CLEAR, L"&Clear Canvas");
    AppendMenuW(g_hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(g_hFileMenu, MF_STRING, ID_FILE_EXIT, L"E&xit");
    AppendMenuW(g_hMenuBar, MF_POPUP, (UINT_PTR)g_hFileMenu, L"&File");

    // Create Tool menu
    g_hToolMenu = CreateMenu();
    AppendMenuW(g_hToolMenu, MF_STRING | MF_CHECKED, ID_TOOL_LINE, L"&Line");
    AppendMenuW(g_hToolMenu, MF_STRING, ID_TOOL_CIRCLE, L"&Circle");
    AppendMenuW(g_hToolMenu, MF_STRING, ID_TOOL_ELLIPSE, L"&Ellipse");
    AppendMenuW(g_hToolMenu, MF_STRING, ID_TOOL_CURVE, L"&Curve");
    AppendMenuW(g_hToolMenu, MF_STRING, ID_TOOL_POLYGON, L"&Polygon");
    AppendMenuW(g_hToolMenu, MF_STRING, ID_TOOL_CLIP, L"C&lipping Window");
    AppendMenuW(g_hMenuBar, MF_POPUP, (UINT_PTR)g_hToolMenu, L"&Tools");

    // Create Algorithm menu
    g_hAlgoMenu = CreateMenu();
    AppendMenuW(g_hAlgoMenu, MF_STRING | MF_CHECKED, ID_ALGO_DDA, L"&DDA");
    AppendMenuW(g_hAlgoMenu, MF_STRING, ID_ALGO_MIDPOINT, L"&Midpoint");
    AppendMenuW(g_hAlgoMenu, MF_STRING, ID_ALGO_PARAMETRIC, L"&Parametric");
    AppendMenuW(g_hAlgoMenu, MF_STRING, ID_ALGO_POLAR, L"P&olar");
    AppendMenuW(g_hAlgoMenu, MF_STRING, ID_ALGO_DIRECT, L"&Direct");
    AppendMenuW(g_hMenuBar, MF_POPUP, (UINT_PTR)g_hAlgoMenu, L"&Algorithms");

    // Create Fill menu
    g_hFillMenu = CreateMenu();
    AppendMenuW(g_hFillMenu, MF_STRING | MF_CHECKED, ID_FILL_LINES, L"Fill with &Lines");
    AppendMenuW(g_hFillMenu, MF_STRING, ID_FILL_CIRCLES, L"Fill with &Circles");
    
    // Add separator before quarter selection options
    AppendMenuW(g_hFillMenu, MF_SEPARATOR, 0, NULL);
    
    // Add quarter selection options directly to the Fill menu instead of a submenu
    AppendMenuW(g_hFillMenu, MF_STRING | MF_CHECKED, ID_QUARTER_FULL, L"Fill &Entire Circle");
    AppendMenuW(g_hFillMenu, MF_STRING, ID_QUARTER_1, L"Fill &Top-Right Quarter");
    AppendMenuW(g_hFillMenu, MF_STRING, ID_QUARTER_2, L"Fill T&op-Left Quarter");
    AppendMenuW(g_hFillMenu, MF_STRING, ID_QUARTER_3, L"Fill &Bottom-Left Quarter");
    AppendMenuW(g_hFillMenu, MF_STRING, ID_QUARTER_4, L"Fill B&ottom-Right Quarter");
    
    AppendMenuW(g_hMenuBar, MF_POPUP, (UINT_PTR)g_hFillMenu, L"&Fill");

    // Create Color menu
    g_hColorMenu = CreateMenu();
    AppendMenuW(g_hColorMenu, MF_STRING | MF_CHECKED, ID_COLOR_BLACK, L"&Black");
    AppendMenuW(g_hColorMenu, MF_STRING, ID_COLOR_RED, L"&Red");
    AppendMenuW(g_hColorMenu, MF_STRING, ID_COLOR_GREEN, L"&Green");
    AppendMenuW(g_hColorMenu, MF_STRING, ID_COLOR_BLUE, L"&Blue");
    AppendMenuW(g_hColorMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(g_hColorMenu, MF_STRING, ID_COLOR_CUSTOM, L"&Custom...");
    AppendMenuW(g_hMenuBar, MF_POPUP, (UINT_PTR)g_hColorMenu, L"&Color");

    // Create Help menu
    g_hHelpMenu = CreateMenu();
    AppendMenuW(g_hHelpMenu, MF_STRING, ID_HELP_ABOUT, L"&About");
    AppendMenuW(g_hMenuBar, MF_POPUP, (UINT_PTR)g_hHelpMenu, L"&Help");

    // Set the menu
    SetMenu(hwnd, g_hMenuBar);
}

void CreateToolbar(HWND hwnd) {
    // Create toolbar window
    g_hToolbar = CreateWindowEx(
        0, TOOLBARCLASSNAME, NULL,
        WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS,
        0, 0, 0, 0,
        hwnd, NULL, GetModuleHandle(NULL), NULL
    );

    // Send the TB_BUTTONSTRUCTSIZE message
    SendMessage(g_hToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

    // Simple toolbar with text buttons
    TBBUTTON tbb[7];
    ZeroMemory(tbb, sizeof(tbb));

    // Line button
    tbb[0].idCommand = ID_TOOL_LINE;
    tbb[0].fsState = TBSTATE_ENABLED;
    tbb[0].fsStyle = BTNS_BUTTON | BTNS_AUTOSIZE;
    tbb[0].iString = (INT_PTR)L"Line";

    // Circle button
    tbb[1].idCommand = ID_TOOL_CIRCLE;
    tbb[1].fsState = TBSTATE_ENABLED;
    tbb[1].fsStyle = BTNS_BUTTON | BTNS_AUTOSIZE;
    tbb[1].iString = (INT_PTR)L"Circle";

    // Ellipse button
    tbb[2].idCommand = ID_TOOL_ELLIPSE;
    tbb[2].fsState = TBSTATE_ENABLED;
    tbb[2].fsStyle = BTNS_BUTTON | BTNS_AUTOSIZE;
    tbb[2].iString = (INT_PTR)L"Ellipse";

    // Curve button
    tbb[3].idCommand = ID_TOOL_CURVE;
    tbb[3].fsState = TBSTATE_ENABLED;
    tbb[3].fsStyle = BTNS_BUTTON | BTNS_AUTOSIZE;
    tbb[3].iString = (INT_PTR)L"Curve";

    // Polygon button
    tbb[4].idCommand = ID_TOOL_POLYGON;
    tbb[4].fsState = TBSTATE_ENABLED;
    tbb[4].fsStyle = BTNS_BUTTON | BTNS_AUTOSIZE;
    tbb[4].iString = (INT_PTR)L"Polygon";

    // Clipping button
    tbb[5].idCommand = ID_TOOL_CLIP;
    tbb[5].fsState = TBSTATE_ENABLED;
    tbb[5].fsStyle = BTNS_BUTTON | BTNS_AUTOSIZE;
    tbb[5].iString = (INT_PTR)L"Clipping";

    // Fill button
    tbb[6].idCommand = ID_TOOL_FILL;
    tbb[6].fsState = TBSTATE_ENABLED;
    tbb[6].fsStyle = BTNS_BUTTON | BTNS_AUTOSIZE;
    tbb[6].iString = (INT_PTR)L"Fill";

    // Add buttons to toolbar
    SendMessage(g_hToolbar, TB_ADDSTRING, 0, (LPARAM)L"Line\0Circle\0Ellipse\0Curve\0Polygon\0Clipping\0Fill\0\0");
    SendMessage(g_hToolbar, TB_ADDBUTTONS, 7, (LPARAM)&tbb);

    // Make toolbar visible
    ShowWindow(g_hToolbar, SW_SHOW);
}

void CreateStatusBar(HWND hwnd) {
    // Create status bar
    g_hStatusBar = CreateWindowEx(
        0, STATUSCLASSNAME, NULL,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0,
        hwnd, NULL, GetModuleHandle(NULL), NULL
    );

    // Set initial status with instructions
    UpdateInstructions();
}

void CreateControls(HWND hwnd) {
    // Get client area dimensions
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);
    
    // Create a panel area for controls
    int controlPanelHeight = CONTROLS_HEIGHT;
    int labelWidth = 80;
    int controlWidth = 180;
    int controlHeight = 24;
    int margin = 15;
    int startX = 20;
    int startY = 20;
    
    // Create labels
    g_hShapeLabel = CreateWindowW(
        L"STATIC", L"Shape:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        startX, startY, labelWidth, controlHeight,
        hwnd, (HMENU)ID_LABEL_SHAPE, GetModuleHandle(NULL), NULL
    );
    
    g_hAlgorithmLabel = CreateWindowW(
        L"STATIC", L"Algorithm:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        startX + labelWidth + controlWidth + margin, startY, labelWidth, controlHeight,
        hwnd, (HMENU)ID_LABEL_ALGORITHM, GetModuleHandle(NULL), NULL
    );
    
    g_hColorLabel = CreateWindowW(
        L"STATIC", L"Color:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        startX, startY + controlHeight + margin, labelWidth, controlHeight,
        hwnd, (HMENU)ID_LABEL_COLOR, GetModuleHandle(NULL), NULL
    );
    
    g_hThicknessLabel = CreateWindowW(
        L"STATIC", L"Thickness:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        startX + labelWidth + controlWidth + margin, startY + controlHeight + margin, labelWidth, controlHeight,
        hwnd, (HMENU)ID_LABEL_THICKNESS, GetModuleHandle(NULL), NULL
    );
    
    // Replace fill method dropdown with a label for fill options and checkbox
    g_hFillMethodLabel = CreateWindowW(
        L"STATIC", L"Fill Options:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        startX, startY + (controlHeight + margin) * 2, labelWidth, controlHeight,
        hwnd, (HMENU)(ID_LABEL_THICKNESS + 2), GetModuleHandle(NULL), NULL
    );
    
    // Create fill checkbox - not checked by default
    g_hFillCheckbox = CreateWindowW(
        L"BUTTON", L"Fill Shapes",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        startX + labelWidth, startY + (controlHeight + margin) * 2, controlWidth, controlHeight,
        hwnd, (HMENU)ID_FILL_CHECKBOX, GetModuleHandle(NULL), NULL
    );
    
    // Initialize checkbox to match g_isFilled state (which is false by default)
    SendMessage(g_hFillCheckbox, BM_SETCHECK, g_isFilled ? BST_CHECKED : BST_UNCHECKED, 0);
    
    // Create help text window in the top right
    g_hHelpText = CreateWindowW(
        L"STATIC", L"",
        WS_CHILD | WS_VISIBLE | SS_LEFT | SS_SUNKEN,
        rcClient.right - 300, startY, 280, 70,
        hwnd, NULL, GetModuleHandle(NULL), NULL
    );
    
    // Set font for labels
    SendMessage(g_hShapeLabel, WM_SETFONT, (WPARAM)g_hLabelFont, TRUE);
    SendMessage(g_hAlgorithmLabel, WM_SETFONT, (WPARAM)g_hLabelFont, TRUE);
    SendMessage(g_hColorLabel, WM_SETFONT, (WPARAM)g_hLabelFont, TRUE);
    SendMessage(g_hThicknessLabel, WM_SETFONT, (WPARAM)g_hLabelFont, TRUE);
    SendMessage(g_hFillMethodLabel, WM_SETFONT, (WPARAM)g_hLabelFont, TRUE);
    SendMessage(g_hHelpText, WM_SETFONT, (WPARAM)g_hLabelFont, TRUE);
    
    // Create shape dropdown
    g_hShapeCombo = CreateWindowW(
        L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        startX + labelWidth, startY, controlWidth, 200,
        hwnd, (HMENU)ID_COMBO_SHAPE, GetModuleHandle(NULL), NULL
    );
    
    // Add shape items
    SendMessage(g_hShapeCombo, CB_ADDSTRING, 0, (LPARAM)L"Line");
    SendMessage(g_hShapeCombo, CB_ADDSTRING, 0, (LPARAM)L"Circle");
    SendMessage(g_hShapeCombo, CB_ADDSTRING, 0, (LPARAM)L"Ellipse");
    SendMessage(g_hShapeCombo, CB_ADDSTRING, 0, (LPARAM)L"Curve");
    SendMessage(g_hShapeCombo, CB_ADDSTRING, 0, (LPARAM)L"Polygon");
    SendMessage(g_hShapeCombo, CB_ADDSTRING, 0, (LPARAM)L"Clipping");
    SendMessage(g_hShapeCombo, CB_ADDSTRING, 0, (LPARAM)L"Fill");
    SendMessage(g_hShapeCombo, CB_SETCURSEL, 0, 0); // Select Line by default
    
    // Create algorithm dropdown
    g_hAlgorithmCombo = CreateWindowW(
        L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        startX + labelWidth + controlWidth + margin + labelWidth, startY, controlWidth, 200,
        hwnd, (HMENU)ID_COMBO_ALGORITHM, GetModuleHandle(NULL), NULL
    );
    
    // Initialize algorithm dropdown based on the default tool (Line)
    UpdateAlgorithmDropdown();
    
    // Create color button
    g_hColorButton = CreateWindowW(
        L"BUTTON", L"Choose Color",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        startX + labelWidth, startY + controlHeight + margin, controlWidth, controlHeight,
        hwnd, (HMENU)ID_COLOR_BUTTON, GetModuleHandle(NULL), NULL
    );
    
    // Create thickness slider
    g_hThicknessSlider = CreateWindowW(
        TRACKBAR_CLASS, L"",
        WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS,
        startX + labelWidth + controlWidth + margin + labelWidth, startY + controlHeight + margin, controlWidth, controlHeight,
        hwnd, (HMENU)ID_THICKNESS_SLIDER, GetModuleHandle(NULL), NULL
    );
    
    // Replace fill method dropdown with a label for fill options and checkbox
    g_hFillMethodLabel = CreateWindowW(
        L"STATIC", L"Fill Options:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        startX, startY + (controlHeight + margin) * 2, labelWidth, controlHeight,
        hwnd, (HMENU)(ID_LABEL_THICKNESS + 2), GetModuleHandle(NULL), NULL
    );
    
    // Create fill checkbox - not checked by default
    g_hFillCheckbox = CreateWindowW(
        L"BUTTON", L"Fill Shapes",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        startX + labelWidth, startY + (controlHeight + margin) * 2, controlWidth, controlHeight,
        hwnd, (HMENU)ID_FILL_CHECKBOX, GetModuleHandle(NULL), NULL
    );
    
    // Initialize checkbox to match g_isFilled state (which is false by default)
    SendMessage(g_hFillCheckbox, BM_SETCHECK, g_isFilled ? BST_CHECKED : BST_UNCHECKED, 0);
    
    // Create quarter selection label - positioned below the fill options
    g_hQuarterLabel = CreateWindowW(
        L"STATIC", L"Quarter:",
        WS_CHILD | SS_LEFT, // Initially hidden
        startX, startY + (controlHeight + margin) * 3, labelWidth, controlHeight,
        hwnd, (HMENU)(ID_LABEL_THICKNESS + 3), GetModuleHandle(NULL), NULL
    );
    
    // Create quarter selection dropdown
    g_hQuarterCombo = CreateWindowW(
        L"COMBOBOX", L"",
        WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL, // Initially hidden
        startX + labelWidth, startY + (controlHeight + margin) * 3, controlWidth, 200,
        hwnd, (HMENU)(ID_COMBO_ALGORITHM + 2), GetModuleHandle(NULL), NULL
    );
    
    // Add quarter selection options
    SendMessage(g_hQuarterCombo, CB_ADDSTRING, 0, (LPARAM)L"Full Circle");
    SendMessage(g_hQuarterCombo, CB_ADDSTRING, 0, (LPARAM)L"Top-Right (1st Quarter)");
    SendMessage(g_hQuarterCombo, CB_ADDSTRING, 0, (LPARAM)L"Top-Left (2nd Quarter)");
    SendMessage(g_hQuarterCombo, CB_ADDSTRING, 0, (LPARAM)L"Bottom-Left (3rd Quarter)");
    SendMessage(g_hQuarterCombo, CB_ADDSTRING, 0, (LPARAM)L"Bottom-Right (4th Quarter)");
    SendMessage(g_hQuarterCombo, CB_SETCURSEL, 0, 0); // Select Full Circle by default
    
    // Create polygon fill options - initially hidden
    g_hPolyFillLabel = CreateWindowW(
        L"STATIC", L"Polygon Fill:",
        WS_CHILD | SS_LEFT, // Initially hidden
        startX, startY + (controlHeight + margin) * 3, labelWidth, controlHeight,
        hwnd, (HMENU)ID_POLY_FILL_LABEL, GetModuleHandle(NULL), NULL
    );
    
    // Create polygon fill dropdown
    g_hPolyFillCombo = CreateWindowW(
        L"COMBOBOX", L"",
        WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL, // Initially hidden
        startX + labelWidth, startY + (controlHeight + margin) * 3, controlWidth, 200,
        hwnd, (HMENU)ID_POLY_FILL_COMBO, GetModuleHandle(NULL), NULL
    );
    
    // Add polygon fill options
    SendMessage(g_hPolyFillCombo, CB_ADDSTRING, 0, (LPARAM)L"Convex Fill");
    SendMessage(g_hPolyFillCombo, CB_ADDSTRING, 0, (LPARAM)L"General Fill");
    SendMessage(g_hPolyFillCombo, CB_ADDSTRING, 0, (LPARAM)L"Flood Fill");
    SendMessage(g_hPolyFillCombo, CB_SETCURSEL, 0, 0); // Select Convex Fill by default
    
    // Initially hide polygon fill options
    ShowWindow(g_hPolyFillLabel, SW_HIDE);
    ShowWindow(g_hPolyFillCombo, SW_HIDE);
    
    // Set font for controls
    SendMessage(g_hShapeCombo, WM_SETFONT, (WPARAM)g_hLabelFont, TRUE);
    SendMessage(g_hAlgorithmCombo, WM_SETFONT, (WPARAM)g_hLabelFont, TRUE);
    SendMessage(g_hQuarterLabel, WM_SETFONT, (WPARAM)g_hLabelFont, TRUE); // Set font for quarter label
    SendMessage(g_hQuarterCombo, WM_SETFONT, (WPARAM)g_hLabelFont, TRUE); // Set font for quarter combo
    SendMessage(g_hColorButton, WM_SETFONT, (WPARAM)g_hButtonFont, TRUE);
    SendMessage(g_hFillCheckbox, WM_SETFONT, (WPARAM)g_hLabelFont, TRUE); // Set font for checkbox
    SendMessage(g_hPolyFillLabel, WM_SETFONT, (WPARAM)g_hLabelFont, TRUE); // Set font for polygon fill label
    SendMessage(g_hPolyFillCombo, WM_SETFONT, (WPARAM)g_hLabelFont, TRUE); // Set font for polygon fill combo
    
    // Set range and position for thickness slider
    SendMessage(g_hThicknessSlider, TBM_SETRANGE, TRUE, MAKELONG(1, 10));
    SendMessage(g_hThicknessSlider, TBM_SETPOS, TRUE, g_lineThickness);
    
    // Update color button appearance
    UpdateColorButton();
    
    // Update control visibility based on current tool
    UpdateControlVisibility();
}

void UpdateStatusBar(const wchar_t* message) {
    SendMessage(g_hStatusBar, SB_SETTEXT, 0, (LPARAM)message);
}

void UpdateColorButton() {
    // Create a color name string based on the current color
    wchar_t colorName[32];
    
    if (g_currentColor == COLOR_BLACK)
        wcscpy(colorName, L"Black");
    else if (g_currentColor == COLOR_RED)
        wcscpy(colorName, L"Red");
    else if (g_currentColor == COLOR_GREEN)
        wcscpy(colorName, L"Green");
    else if (g_currentColor == COLOR_BLUE)
        wcscpy(colorName, L"Blue");
    else
        wcscpy(colorName, L"Custom");
    
    // Set the button text
    SetWindowTextW(g_hColorButton, colorName);
    
    // Redraw the button
    InvalidateRect(g_hColorButton, NULL, TRUE);
}

void InitializeDrawingBuffer(HWND hwnd) {
    // Get the canvas area
    g_canvasRect = GetCanvasRect(hwnd);
    
    // Get the device context
    HDC hdc = GetDC(hwnd);
    
    // Create a compatible DC
    g_hdcMem = CreateCompatibleDC(hdc);
    
    // Create a compatible bitmap
    g_hbmMem = CreateCompatibleBitmap(hdc, g_canvasRect.right - g_canvasRect.left, 
                                     g_canvasRect.bottom - g_canvasRect.top);
    
    // Select the bitmap into the memory DC
    SelectObject(g_hdcMem, g_hbmMem);
    
    // Fill with white
    RECT rc = {0, 0, g_canvasRect.right - g_canvasRect.left, g_canvasRect.bottom - g_canvasRect.top};
    FillRect(g_hdcMem, &rc, g_hCanvasBrush);
    
    // Release the device context
    ReleaseDC(hwnd, hdc);
}

void CleanupDrawingBuffer() {
    if (g_hdcMem) {
        DeleteDC(g_hdcMem);
        g_hdcMem = NULL;
    }
    
    if (g_hbmMem) {
        DeleteObject(g_hbmMem);
        g_hbmMem = NULL;
    }
}

RECT GetCanvasRect(HWND hwnd) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    
    // Adjust for controls panel and status bar
    rc.top += CONTROLS_HEIGHT + CANVAS_MARGIN;
    rc.left += CANVAS_MARGIN;
    rc.right -= CANVAS_MARGIN;
    rc.bottom -= (STATUSBAR_HEIGHT + CANVAS_MARGIN);
    
    return rc;
}

void HandleMenuSelection(HWND hwnd, WPARAM wParam) {
    // Variable declarations
    int fillIndex = 0;
    
    // Handle menu selections
    switch (LOWORD(wParam)) {
        case ID_FILE_EXIT:
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            break;

        case ID_FILE_OPEN:
            ShellExecuteW(NULL, L"open", g_htmlPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
            break;

        case ID_FILE_SAVE:
            SaveCanvasToBitmap(hwnd);
            break;

        case ID_FILE_CLEAR:
            ClearCanvas(g_hdcMem);
            InvalidateRect(hwnd, &g_canvasRect, FALSE);
            break;

        // Tool selection
        case ID_TOOL_LINE:
        case ID_TOOL_CIRCLE:
        case ID_TOOL_ELLIPSE:
        case ID_TOOL_CURVE:
        case ID_TOOL_POLYGON:
        case ID_TOOL_CLIP:
            // Uncheck previous tool
            CheckMenuItem(g_hToolMenu, g_currentTool, MF_BYCOMMAND | MF_UNCHECKED);
            // Update current tool
            g_currentTool = LOWORD(wParam);
            // Check new tool
            CheckMenuItem(g_hToolMenu, g_currentTool, MF_BYCOMMAND | MF_CHECKED);
            
            // Reset polygon points if needed
            if (g_currentTool != ID_TOOL_POLYGON) {
                g_polygonPoints.clear();
            }
            
            // Reset ellipse click counter and drawing mode if changing from ellipse
            if (g_currentTool != ID_TOOL_ELLIPSE) {
                g_ellipseClickCount = 0;
                g_ellipseDrawingMode = false;
                
                // Reset all ellipse points
                g_ellipseCenter.x = 0;
                g_ellipseCenter.y = 0;
                g_ellipsePoint1.x = 0;
                g_ellipsePoint1.y = 0;
                g_ellipsePoint2.x = 0;
                g_ellipsePoint2.y = 0;
                g_ellipseA = 0;
                g_ellipseB = 0;
            } else {
                // If we're switching to ellipse, set the drawing mode
                g_ellipseDrawingMode = true;
            }
            
            // Update the algorithm dropdown to show appropriate algorithms for this tool
            UpdateAlgorithmDropdown();
            
            // Update quarter selection visibility based on current tool
            UpdateControlVisibility();
            
            // Update the UI controls to match the selection
            UpdateControlsFromSelection();
            
            // Update instructions in the status bar
            UpdateInstructions();
            break;

        // Algorithm selection
        case ID_ALGO_DDA:
        case ID_ALGO_MIDPOINT:
        case ID_ALGO_PARAMETRIC:
        case ID_ALGO_POLAR:
        case ID_ALGO_DIRECT:
        case ID_ALGO_BEZIER:
        case ID_ALGO_HERMITE:
        case ID_ALGO_COHEN_SUTHERLAND:
        case ID_ALGO_LIANG_BARSKY:
            {
                // Uncheck previous algorithm
                CheckMenuItem(g_hAlgoMenu, g_currentAlgorithm, MF_BYCOMMAND | MF_UNCHECKED);
                // Update current algorithm
                g_currentAlgorithm = LOWORD(wParam);
                // Check new algorithm
                CheckMenuItem(g_hAlgoMenu, g_currentAlgorithm, MF_BYCOMMAND | MF_CHECKED);
                
                // Explicitly update the algorithm combo box selection
                int algoIndex = 0;
                switch (g_currentTool) {
                    case ID_TOOL_LINE:
                        switch (g_currentAlgorithm) {
                            case ID_ALGO_DDA: algoIndex = 0; break;
                            case ID_ALGO_MIDPOINT: algoIndex = 1; break;
                            case ID_ALGO_PARAMETRIC: algoIndex = 2; break;
                        }
                        break;
                        
                    case ID_TOOL_CIRCLE:
                        switch (g_currentAlgorithm) {
                            case ID_ALGO_MIDPOINT: algoIndex = 0; break;
                            case ID_ALGO_POLAR: algoIndex = 1; break;
                            case ID_ALGO_PARAMETRIC: algoIndex = 2; break;
                        }
                        break;
                        
                    case ID_TOOL_ELLIPSE:
                        switch (g_currentAlgorithm) {
                            case ID_ALGO_MIDPOINT: algoIndex = 0; break;
                            case ID_ALGO_DIRECT: algoIndex = 1; break;
                            case ID_ALGO_POLAR: algoIndex = 2; break;
                        }
                        break;
                        
                    case ID_TOOL_CURVE:
                        switch (g_currentAlgorithm) {
                            case ID_ALGO_CARDINAL: algoIndex = 0; break;
                            default: algoIndex = 0; break;
                        }
                        break;
                        
                    case ID_TOOL_POLYGON:
                        switch (g_currentAlgorithm) {
                            case ID_ALGO_DDA: algoIndex = 0; break;
                            case ID_ALGO_MIDPOINT: algoIndex = 1; break;
                        }
                        break;
                        
                    case ID_TOOL_CLIP:
                        switch (g_currentAlgorithm) {
                            case ID_ALGO_COHEN_SUTHERLAND: algoIndex = 0; break;
                            case ID_ALGO_LIANG_BARSKY: algoIndex = 1; break;
                        }
                        break;
                }
                SendMessage(g_hAlgorithmCombo, CB_SETCURSEL, algoIndex, 0);
                
                // Force redraw of the algorithm combo box
                InvalidateRect(g_hAlgorithmCombo, NULL, TRUE);
                UpdateWindow(g_hAlgorithmCombo);
                
                // Update instructions
                UpdateInstructions();
            }
            break;

        // Fill selection
        case ID_FILL_LINES:
        case ID_FILL_CIRCLES:
            // No longer needed - we use radio buttons now
            break;
            
        // Quarter selection for circle filling
        case ID_QUARTER_FULL:
        case ID_QUARTER_1:
        case ID_QUARTER_2:
        case ID_QUARTER_3:
        case ID_QUARTER_4:
            // Uncheck previous quarter selection
            CheckMenuItem(NULL, g_selectedQuarter, MF_BYCOMMAND | MF_UNCHECKED);
            // Update current quarter
            g_selectedQuarter = LOWORD(wParam);
            // Check new quarter
            CheckMenuItem(NULL, g_selectedQuarter, MF_BYCOMMAND | MF_CHECKED);
            // Update status bar with selection
            wchar_t quarterMsg[256];
            if (g_selectedQuarter == ID_QUARTER_FULL)
                _snwprintf(quarterMsg, 256, L"Selected to fill the entire circle");
            else
                _snwprintf(quarterMsg, 256, L"Selected to fill quarter %d of circle", g_selectedQuarter - ID_QUARTER_1 + 1);
            UpdateStatusBar(quarterMsg);
            break;

        // Color selection
        case ID_COLOR_BLACK:
        case ID_COLOR_RED:
        case ID_COLOR_GREEN:
        case ID_COLOR_BLUE:
            // Uncheck previous color
            CheckMenuItem(g_hColorMenu, ID_COLOR_BLACK, MF_BYCOMMAND | MF_UNCHECKED);
            CheckMenuItem(g_hColorMenu, ID_COLOR_RED, MF_BYCOMMAND | MF_UNCHECKED);
            CheckMenuItem(g_hColorMenu, ID_COLOR_GREEN, MF_BYCOMMAND | MF_UNCHECKED);
            CheckMenuItem(g_hColorMenu, ID_COLOR_BLUE, MF_BYCOMMAND | MF_UNCHECKED);
            
            // Set new color
            switch (LOWORD(wParam)) {
                case ID_COLOR_BLACK:
                    g_currentColor = COLOR_BLACK;
                    break;
                case ID_COLOR_RED:
                    g_currentColor = COLOR_RED;
                    break;
                case ID_COLOR_GREEN:
                    g_currentColor = COLOR_GREEN;
                    break;
                case ID_COLOR_BLUE:
                    g_currentColor = COLOR_BLUE;
                    break;
            }
            
            // Check new color
            CheckMenuItem(g_hColorMenu, LOWORD(wParam), MF_BYCOMMAND | MF_CHECKED);
            UpdateColorButton();
            break;

        case ID_COLOR_CUSTOM:
            ShowColorDialog(hwnd);
            break;

        case ID_HELP_ABOUT:
            MessageBoxW(hwnd, L"PixelCanvas - Drawing Application\n\n"
                        L"A computer graphics program implementing various drawing algorithms.\n\n"
                        L"© 2023 FCAI Computer Graphics", L"About PixelCanvas", MB_OK | MB_ICONINFORMATION);
            break;
    }

    // Update status bar with current selections
    wchar_t statusText[256];
    const wchar_t* toolName = 
        (g_currentTool == ID_TOOL_LINE) ? L"Line" :
        (g_currentTool == ID_TOOL_CIRCLE) ? L"Circle" :
        (g_currentTool == ID_TOOL_ELLIPSE) ? L"Ellipse" :
        (g_currentTool == ID_TOOL_CURVE) ? L"Curve" :
        (g_currentTool == ID_TOOL_POLYGON) ? L"Polygon" : L"Clipping";
    
    const wchar_t* algoName;
    switch (g_currentAlgorithm) {
        case ID_ALGO_DDA: algoName = L"DDA"; break;
        case ID_ALGO_MIDPOINT: algoName = L"Midpoint"; break;
        case ID_ALGO_PARAMETRIC: algoName = L"Parametric"; break;
        case ID_ALGO_POLAR: algoName = L"Polar"; break;
        case ID_ALGO_BEZIER: algoName = L"Bezier"; break;
        case ID_ALGO_HERMITE: algoName = L"Hermite"; break;
        case ID_ALGO_CARDINAL: algoName = L"Cardinal"; break;
        case ID_ALGO_COHEN_SUTHERLAND: algoName = L"Cohen-Sutherland"; break;
        case ID_ALGO_LIANG_BARSKY: algoName = L"Liang-Barsky"; break;
        default: algoName = L"Unknown"; break;
    }
    
    _snwprintf(statusText, 256, L"Current tool: %s, Algorithm: %s, Thickness: %d", 
             toolName, algoName, g_lineThickness);
    UpdateStatusBar(statusText);
}

void DrawLine(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color, int thickness) {
    // Line drawing should never try to use fill methods, regardless of g_currentFill setting
    try {
        // Create Line object using the Line class
        Line line(hdc);
        
        // Choose algorithm based on g_currentAlgorithm
        switch (g_currentAlgorithm) {
            case ID_ALGO_DDA:
                if (thickness == 1) {
                    // Use the Line class implementation for DDA
                    line.DrawLineDDA(x1, y1, x2, y2, color);
                } else {
                    // Use GDI for thicker lines
                    HPEN hPen = CreatePen(PS_SOLID, thickness, color);
                    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
                    MoveToEx(hdc, x1, y1, NULL);
                    LineTo(hdc, x2, y2);
                    SelectObject(hdc, hOldPen);
                    DeleteObject(hPen);
                }
                break;
                
            case ID_ALGO_MIDPOINT:
                if (thickness == 1) {
                    // Use the Line class implementation for Midpoint
                    line.DrawLineMidpoint(x1, y1, x2, y2, color);
                } else {
                    // Fall back to GDI for thicker lines
                    HPEN hPen = CreatePen(PS_SOLID, thickness, color);
                    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
                    MoveToEx(hdc, x1, y1, NULL);
                    LineTo(hdc, x2, y2);
                    SelectObject(hdc, hOldPen);
                    DeleteObject(hPen);
                }
                break;
                
            case ID_ALGO_PARAMETRIC:
                if (thickness == 1) {
                    // Use the Line class implementation for Parametric
                    line.DrawLineParametric(x1, y1, x2, y2, color);
                } else {
                    // Fall back to GDI for thicker lines
                    HPEN hPen = CreatePen(PS_SOLID, thickness, color);
                    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
                    MoveToEx(hdc, x1, y1, NULL);
                    LineTo(hdc, x2, y2);
                    SelectObject(hdc, hOldPen);
                    DeleteObject(hPen);
                }
                break;
                
            default:
                // Default to DDA for unsupported algorithms
                if (thickness == 1) {
                    line.DrawLineDDA(x1, y1, x2, y2, color);
                } else {
                    // Fall back to GDI for thicker lines
                    HPEN hPen = CreatePen(PS_SOLID, thickness, color);
                    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
                    MoveToEx(hdc, x1, y1, NULL);
                    LineTo(hdc, x2, y2);
                    SelectObject(hdc, hOldPen);
                    DeleteObject(hPen);
                }
                break;
        }
    }
    catch (...) {
        // Fallback to simpler drawing if needed
        SetPixel(hdc, x1, y1, color);
        SetPixel(hdc, x2, y2, color);
    }
}

void DrawCircle(HDC hdc, int centerX, int centerY, int radius, COLORREF color, int thickness, bool filled) {
    // Create a Circle object using the implementation in Circle.cpp
    Circle circle(hdc);
    
    if (filled) {
        // For filled circles, use the Circle class's built-in fill methods
        // First draw the outline
        circle.DrawCircleMidpoint(centerX, centerY, radius, color);
        
        // Then fill it based on the selected quarter
        if (g_selectedQuarter == ID_QUARTER_FULL) {
            // Fill the entire circle - alternate between line and circle fills
            static bool useCircleFill = false;
            useCircleFill = !useCircleFill;
            
            if (useCircleFill) {
                // Fill with concentric circles
                circle.FillWithCircles(centerX, centerY, radius);
            } else {
                // Fill with lines
                circle.FillWithLines(centerX, centerY, radius, color);
            }
        } else if (g_selectedQuarter == ID_QUARTER_1) {
            circle.FillQuarterWithCircles(centerX, centerY, radius, 1);
        } else if (g_selectedQuarter == ID_QUARTER_2) {
            circle.FillQuarterWithCircles(centerX, centerY, radius, 2);
        } else if (g_selectedQuarter == ID_QUARTER_3) {
            circle.FillQuarterWithCircles(centerX, centerY, radius, 3);
        } else if (g_selectedQuarter == ID_QUARTER_4) {
            circle.FillQuarterWithCircles(centerX, centerY, radius, 4);
        }
    } else {
        // For outline only, use the Circle class's drawing method based on the selected algorithm
        if (g_currentAlgorithm == ID_ALGO_MIDPOINT) {
            circle.DrawCircleMidpoint(centerX, centerY, radius, color);
        } else if (g_currentAlgorithm == ID_ALGO_POLAR) {
            circle.DrawCirclePolar(centerX, centerY, radius, color);
        } else {
            // Default to midpoint if no specific algorithm is matched
            circle.DrawCircleMidpoint(centerX, centerY, radius, color);
        }
    }
}

void DrawEllipse(HDC hdc, int centerX, int centerY, int radiusX, int radiusY, COLORREF color, int thickness, bool filled)
{
    // Use standard GDI drawing for ellipses
    if (filled) {
        // Fill the ellipse using standard GDI
        HBRUSH hBrush = CreateSolidBrush(color);
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
        HPEN hPen = CreatePen(PS_SOLID, 1, color);
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        
        Ellipse(hdc, centerX - radiusX, centerY - radiusY, centerX + radiusX, centerY + radiusY);
        
        SelectObject(hdc, hOldBrush);
        SelectObject(hdc, hOldPen);
        DeleteObject(hBrush);
        DeleteObject(hPen);
    } else {
        // For outline, use specified thickness
        HPEN hPen = CreatePen(PS_SOLID, thickness, color);
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
        
        Ellipse(hdc, centerX - radiusX, centerY - radiusY, centerX + radiusX, centerY + radiusY);
        
        SelectObject(hdc, hOldPen);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hPen);
    }
}

void ClearCanvas(HDC hdc) {
    // Get the size of the drawing area
    int width = g_canvasRect.right - g_canvasRect.left;
    int height = g_canvasRect.bottom - g_canvasRect.top;
    
    // Fill with white
    RECT rc = {0, 0, width, height};
    FillRect(hdc, &rc, g_hCanvasBrush);
}

void SaveCanvasToBitmap(HWND hwnd) {
    // Get the file name from a save dialog
    wchar_t fileName[MAX_PATH] = L"";
    
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"Bitmap Files (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = L"bmp";
    
    if (GetSaveFileNameW(&ofn)) {
        // Get the size of the drawing area
        int width = g_canvasRect.right - g_canvasRect.left;
        int height = g_canvasRect.bottom - g_canvasRect.top;
        
        // Create a bitmap info structure
        BITMAPINFO bmi;
        ZeroMemory(&bmi, sizeof(BITMAPINFO));
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = height;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 24;
        bmi.bmiHeader.biCompression = BI_RGB;
        
        // Create a DIB section
        BYTE* bits;
        HDC hdc = GetDC(hwnd);
        HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (void**)&bits, NULL, 0);
        
        if (hBitmap) {
            // Create a compatible DC
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);
            
            // Copy the canvas to the bitmap
            BitBlt(hdcMem, 0, 0, width, height, g_hdcMem, 0, 0, SRCCOPY);
            
            // Save the bitmap to a file
            HANDLE hFile = CreateFileW(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            
            if (hFile != INVALID_HANDLE_VALUE) {
                // Create file headers
                BITMAPFILEHEADER bmfh;
                ZeroMemory(&bmfh, sizeof(BITMAPFILEHEADER));
                bmfh.bfType = 0x4D42; // "BM"
                bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + width * height * 3;
                bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
                
                // Write the headers and bitmap data
                DWORD bytesWritten;
                WriteFile(hFile, &bmfh, sizeof(BITMAPFILEHEADER), &bytesWritten, NULL);
                WriteFile(hFile, &bmi.bmiHeader, sizeof(BITMAPINFOHEADER), &bytesWritten, NULL);
                
                // Get the DIB bits
                GetDIBits(hdcMem, hBitmap, 0, height, bits, &bmi, DIB_RGB_COLORS);
                
                // Write the bits
                WriteFile(hFile, bits, width * height * 3, &bytesWritten, NULL);
                
                CloseHandle(hFile);
                
                MessageBoxW(hwnd, L"Image saved successfully.", L"Save Complete", MB_OK | MB_ICONINFORMATION);
            }
            
            // Clean up
            SelectObject(hdcMem, hOldBitmap);
            DeleteDC(hdcMem);
            DeleteObject(hBitmap);
        }
        
        ReleaseDC(hwnd, hdc);
    }
}

void ShowColorDialog(HWND hwnd) {
    // Initialize the color dialog structure
    CHOOSECOLOR cc;
    static COLORREF customColors[16];
    
    ZeroMemory(&cc, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = hwnd;
    cc.lpCustColors = customColors;
    cc.rgbResult = g_currentColor;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;
    
    if (ChooseColor(&cc)) {
        // Uncheck previous color menu items
        CheckMenuItem(g_hColorMenu, ID_COLOR_BLACK, MF_BYCOMMAND | MF_UNCHECKED);
        CheckMenuItem(g_hColorMenu, ID_COLOR_RED, MF_BYCOMMAND | MF_UNCHECKED);
        CheckMenuItem(g_hColorMenu, ID_COLOR_GREEN, MF_BYCOMMAND | MF_UNCHECKED);
        CheckMenuItem(g_hColorMenu, ID_COLOR_BLUE, MF_BYCOMMAND | MF_UNCHECKED);
        
        // Set the new color
        g_currentColor = cc.rgbResult;
        
        // Update the color button
        UpdateColorButton();
    }
}

void HandleShapeSelection() {
    // Get the current selection
    int selection = (int)SendMessage(g_hShapeCombo, CB_GETCURSEL, 0, 0);
    
    // Map selection index to tool ID
    int toolID = ID_TOOL_LINE;
    switch (selection) {
        case 0: toolID = ID_TOOL_LINE; break;
        case 1: toolID = ID_TOOL_CIRCLE; break;
        case 2: toolID = ID_TOOL_ELLIPSE; break;
        case 3: toolID = ID_TOOL_CURVE; break;
        case 4: toolID = ID_TOOL_POLYGON; break;
        case 5: toolID = ID_TOOL_CLIP; break;
    }
    
    // Update current tool
    g_currentTool = toolID;
    
    // Reset polygon points if needed
    if (g_currentTool != ID_TOOL_POLYGON) {
        g_polygonPoints.clear();
    }
    
    // Reset ellipse click counter and drawing mode if changing from ellipse
    if (g_currentTool != ID_TOOL_ELLIPSE) {
        g_ellipseClickCount = 0;
        g_ellipseDrawingMode = false;
        
        // Reset all ellipse points
        g_ellipseCenter.x = 0;
        g_ellipseCenter.y = 0;
        g_ellipsePoint1.x = 0;
        g_ellipsePoint1.y = 0;
        g_ellipsePoint2.x = 0;
        g_ellipsePoint2.y = 0;
        g_ellipseA = 0;
        g_ellipseB = 0;
    } else {
        // If we're switching to ellipse, set the drawing mode
        g_ellipseDrawingMode = true;
    }
    
    // Update the algorithm dropdown to show appropriate algorithms for this tool
    UpdateAlgorithmDropdown();
    
    // Update quarter selection and polygon fill visibility based on current tool
    UpdateControlVisibility();
    
    // Update the UI controls to match the selection
    UpdateControlsFromSelection();
    
    // Update instructions in the status bar
    UpdateInstructions();
}

void HandleAlgorithmSelection() {
    // Get the current selection
    int selection = (int)SendMessage(g_hAlgorithmCombo, CB_GETCURSEL, 0, 0);
    
    // Clean up the Circle object if it exists to prevent memory issues
    // if (g_circle != nullptr) {
    //     delete g_circle;
    //     g_circle = nullptr;
    // }
    
    // Map selection index to algorithm ID based on current tool
    int algoID = ID_ALGO_DDA;
    
    switch (g_currentTool) {
        case ID_TOOL_LINE:
            switch (selection) {
                case 0: algoID = ID_ALGO_DDA; break;
                case 1: algoID = ID_ALGO_MIDPOINT; break;
                case 2: algoID = ID_ALGO_PARAMETRIC; break;
                default: algoID = ID_ALGO_DDA; break;
            }
            break;
            
        case ID_TOOL_CIRCLE:
            switch (selection) {
                case 0: algoID = ID_ALGO_MIDPOINT; break;
                case 1: algoID = ID_ALGO_POLAR; break;
                case 2: algoID = ID_ALGO_PARAMETRIC; break;
                default: algoID = ID_ALGO_MIDPOINT; break;
            }
            break;
            
        case ID_TOOL_ELLIPSE:
            switch (selection) {
                case 0: algoID = ID_ALGO_MIDPOINT; break;
                case 1: algoID = ID_ALGO_DIRECT; break;
                case 2: algoID = ID_ALGO_POLAR; break;
                default: algoID = ID_ALGO_MIDPOINT; break;
            }
            break;
            
        case ID_TOOL_CURVE:
            switch (selection) {
                case 0: algoID = ID_ALGO_CARDINAL; break;
                default: algoID = ID_ALGO_CARDINAL; break;
            }
            break;
            
        case ID_TOOL_POLYGON:
            switch (selection) {
                case 0: algoID = ID_ALGO_DDA; break;
                case 1: algoID = ID_ALGO_MIDPOINT; break;
                default: algoID = ID_ALGO_DDA; break;
            }
            break;
            
        case ID_TOOL_CLIP:
            switch (selection) {
                case 0: algoID = ID_ALGO_COHEN_SUTHERLAND; break;
                case 1: algoID = ID_ALGO_LIANG_BARSKY; break;
                default: algoID = ID_ALGO_COHEN_SUTHERLAND; break;
            }
            break;
    }
    
    // Uncheck previous algorithm in menu
    CheckMenuItem(g_hAlgoMenu, g_currentAlgorithm, MF_BYCOMMAND | MF_UNCHECKED);
    
    // Update current algorithm
    g_currentAlgorithm = algoID;
    
    // Check new algorithm in menu
    CheckMenuItem(g_hAlgoMenu, g_currentAlgorithm, MF_BYCOMMAND | MF_CHECKED);
    
    // Update instructions in the status bar
    UpdateInstructions();
    
    // Reset click counter for next ellipse
    g_ellipseClickCount = 0;

    // Reset all ellipse points to avoid interference with the next ellipse
    g_ellipseCenter.x = 0;
    g_ellipseCenter.y = 0;
    g_ellipsePoint1.x = 0;
    g_ellipsePoint1.y = 0;
    g_ellipsePoint2.x = 0;
    g_ellipsePoint2.y = 0;
    g_ellipseA = 0;
    g_ellipseB = 0;

    // Show/hide Cardinal Spline controls
    UpdateCardinalControls(g_hwnd, g_currentAlgorithm == ID_ALGO_CARDINAL);
}

void HandleFillMethodSelection() {
    // No longer needed - UI uses radio buttons now that directly update g_isFilled
}

void HandleQuarterSelection() {
    // Get the current selection index
    int selection = (int)SendMessage(g_hQuarterCombo, CB_GETCURSEL, 0, 0);
    
    // Clean up the Circle object if it exists to prevent memory issues
    // if (g_circle != nullptr) {
    //     delete g_circle;
    //     g_circle = nullptr;
    // }
    
    // Map selection index to quarter ID
    switch (selection) {
        case 0:  // Full Circle
            g_selectedQuarter = ID_QUARTER_FULL;
            break;
        case 1:  // Top-Right (1st Quarter)
            g_selectedQuarter = ID_QUARTER_1;
            break;
        case 2:  // Top-Left (2nd Quarter)
            g_selectedQuarter = ID_QUARTER_2;
            break;
        case 3:  // Bottom-Left (3rd Quarter)
            g_selectedQuarter = ID_QUARTER_3;
            break;
        case 4:  // Bottom-Right (4th Quarter)
            g_selectedQuarter = ID_QUARTER_4;
            break;
        default:
            g_selectedQuarter = ID_QUARTER_FULL;
            break;
    }
    
    // Update status bar with selected quarter
    wchar_t statusText[256];
    if (g_selectedQuarter == ID_QUARTER_FULL) {
        _snwprintf(statusText, 256, L"Selected to fill the entire circle");
    } else {
        _snwprintf(statusText, 256, L"Selected to fill quarter %d of circle", 
                  g_selectedQuarter - ID_QUARTER_1 + 1);
    }
    UpdateStatusBar(statusText);
}

void UpdateControlsFromSelection() {
    // Update the shape dropdown based on current tool
    int shapeIndex = 0;
    switch (g_currentTool) {
        case ID_TOOL_LINE: shapeIndex = 0; break;
        case ID_TOOL_CIRCLE: shapeIndex = 1; break;
        case ID_TOOL_ELLIPSE: shapeIndex = 2; break;
        case ID_TOOL_CURVE: shapeIndex = 3; break;
        case ID_TOOL_POLYGON: shapeIndex = 4; break;
        case ID_TOOL_CLIP: shapeIndex = 5; break;
        case ID_TOOL_FILL: shapeIndex = 6; break;
    }
    SendMessage(g_hShapeCombo, CB_SETCURSEL, shapeIndex, 0);
    
    // Update the algorithm dropdown based on current algorithm
    int algoIndex = 0;
    switch (g_currentAlgorithm) {
        case ID_ALGO_DDA: algoIndex = 0; break;
        case ID_ALGO_MIDPOINT: algoIndex = 1; break;
        case ID_ALGO_PARAMETRIC: algoIndex = 2; break;
        case ID_ALGO_POLAR: algoIndex = 3; break;
        case ID_ALGO_CARDINAL: algoIndex = 4; break;
    }
    SendMessage(g_hAlgorithmCombo, CB_SETCURSEL, algoIndex, 0);
    
    // Update fill checkbox
    SendMessage(g_hFillCheckbox, BM_SETCHECK, g_isFilled ? BST_CHECKED : BST_UNCHECKED, 0);
    
    // Update the thickness slider
    SendMessage(g_hThicknessSlider, TBM_SETPOS, TRUE, g_lineThickness);
    
    // Update the color button
    UpdateColorButton();
}

void UpdateAlgorithmDropdown() {
    // Clear existing items
    SendMessage(g_hAlgorithmCombo, CB_RESETCONTENT, 0, 0);
    
    // Add appropriate algorithms based on current tool
    switch (g_currentTool) {
        case ID_TOOL_LINE:
            SendMessage(g_hAlgorithmCombo, CB_ADDSTRING, 0, (LPARAM)L"DDA");
            SendMessage(g_hAlgorithmCombo, CB_ADDSTRING, 0, (LPARAM)L"Midpoint");
            SendMessage(g_hAlgorithmCombo, CB_ADDSTRING, 0, (LPARAM)L"Parametric");
            break;
            
        case ID_TOOL_CIRCLE:
            SendMessage(g_hAlgorithmCombo, CB_ADDSTRING, 0, (LPARAM)L"Midpoint");
            SendMessage(g_hAlgorithmCombo, CB_ADDSTRING, 0, (LPARAM)L"Polar");
            SendMessage(g_hAlgorithmCombo, CB_ADDSTRING, 0, (LPARAM)L"Parametric");
            break;
            
        case ID_TOOL_ELLIPSE:
            SendMessage(g_hAlgorithmCombo, CB_ADDSTRING, 0, (LPARAM)L"Midpoint");
            SendMessage(g_hAlgorithmCombo, CB_ADDSTRING, 0, (LPARAM)L"Direct");
            SendMessage(g_hAlgorithmCombo, CB_ADDSTRING, 0, (LPARAM)L"Polar");
            break;
            
        case ID_TOOL_CURVE:
            SendMessage(g_hAlgorithmCombo, CB_ADDSTRING, 0, (LPARAM)L"Cardinal");
            break;
            
        case ID_TOOL_POLYGON:
            SendMessage(g_hAlgorithmCombo, CB_ADDSTRING, 0, (LPARAM)L"DDA");
            SendMessage(g_hAlgorithmCombo, CB_ADDSTRING, 0, (LPARAM)L"Midpoint");
            break;
            
        case ID_TOOL_CLIP:
            SendMessage(g_hAlgorithmCombo, CB_ADDSTRING, 0, (LPARAM)L"Cohen-Sutherland");
            SendMessage(g_hAlgorithmCombo, CB_ADDSTRING, 0, (LPARAM)L"Liang-Barsky");
            break;
    }
    
    // Select the first item by default
    SendMessage(g_hAlgorithmCombo, CB_SETCURSEL, 0, 0);
}

void UpdateInstructions() {
    wchar_t statusText[256];
    const wchar_t* toolName = 
        (g_currentTool == ID_TOOL_LINE) ? L"Line" :
        (g_currentTool == ID_TOOL_CIRCLE) ? L"Circle" :
        (g_currentTool == ID_TOOL_ELLIPSE) ? L"Ellipse" :
        (g_currentTool == ID_TOOL_CURVE) ? L"Curve" :
        (g_currentTool == ID_TOOL_POLYGON) ? L"Polygon" :
        (g_currentTool == ID_TOOL_FILL) ? L"Fill" : L"Clipping";
    
    const wchar_t* algoName;
    switch (g_currentAlgorithm) {
        case ID_ALGO_DDA: algoName = L"DDA"; break;
        case ID_ALGO_MIDPOINT: algoName = L"Midpoint"; break;
        case ID_ALGO_PARAMETRIC: algoName = L"Parametric"; break;
        case ID_ALGO_POLAR: algoName = L"Polar"; break;
        case ID_ALGO_BEZIER: algoName = L"Bezier"; break;
        case ID_ALGO_HERMITE: algoName = L"Hermite"; break;
        case ID_ALGO_CARDINAL: algoName = L"Cardinal"; break;
        case ID_ALGO_COHEN_SUTHERLAND: algoName = L"Cohen-Sutherland"; break;
        case ID_ALGO_LIANG_BARSKY: algoName = L"Liang-Barsky"; break;
        default: algoName = L"Unknown"; break;
    }
    
    // Add user-friendly instructions based on the current tool
    const wchar_t* instructions;
    switch (g_currentTool) {
        case ID_TOOL_LINE:
            instructions = L"Click and drag to draw a line";
            break;
        case ID_TOOL_CIRCLE:
            instructions = L"Click at center point and drag to set radius";
            break;
        case ID_TOOL_ELLIPSE:
            if (g_ellipseClickCount == 0)
                instructions = L"Click to place center point";
            else if (g_ellipseClickCount == 1)
                instructions = L"Click to define first axis point";
            else if (g_ellipseClickCount == 2)
                instructions = L"Click to define second axis point";
            else
                instructions = L"Click 3 points: 1) center, 2) first axis, 3) second axis";
            break;
        case ID_TOOL_CURVE:
            instructions = L"Click to place 4 control points for the curve";
            break;
        case ID_TOOL_POLYGON:
            instructions = L"Click for each vertex, right-click to close polygon";
            break;
        case ID_TOOL_CLIP:
            instructions = L"Click and drag to define clipping window";
            break;
        case ID_TOOL_FILL:
            instructions = L"Click inside a closed shape to fill it";
            break;
        default:
            instructions = L"Select a tool to begin drawing";
            break;
    }
    
    _snwprintf(statusText, 256, L"%s: %s. Tool: %s, Algorithm: %s, Thickness: %d", 
             toolName, instructions, toolName, algoName, g_lineThickness);
    UpdateStatusBar(statusText);
    
    // Update help text window with appropriate instructions based on the current tool
    wchar_t helpText[512] = L"";

    switch (g_currentTool) {
        case ID_TOOL_LINE:
            _snwprintf(helpText, 512, 
                    L"How to draw a line:\n"
                    L"1. Click where you want the line to start\n"
                    L"2. Drag to where you want the line to end\n"
                    L"3. Release to draw the line\n\n"
                    L"Current algorithm: %s", algoName);
            break;
            
        case ID_TOOL_CIRCLE:
            _snwprintf(helpText, 512, 
                    L"How to draw a circle:\n"
                    L"1. Click at the center point\n"
                    L"2. Drag to set the radius\n"
                    L"3. Release to draw the circle\n\n"
                    L"Current algorithm: %s\n"
                    L"Note: Use Fill menu > Quarter Selection\n"
                    L"to fill specific quarters of circles", algoName);
            break;
            
        case ID_TOOL_ELLIPSE:
            _snwprintf(helpText, 512, 
                    L"How to draw an ellipse:\n"
                    L"1. Click to set the center point\n"
                    L"2. Click to define the first axis point\n"
                    L"3. Click to define the second axis point\n\n"
                    L"Current algorithm: %s", algoName);
            break;
            
        case ID_TOOL_CURVE:
            _snwprintf(helpText, 512, 
                    L"How to draw a Cardinal Spline curve:\n"
                    L"1. Click to place exactly 4 control points\n"
                    L"2. After placing the 4th point, the curve will\n   be drawn automatically\n"
                    L"3. Click anywhere to start a new curve\n\n"
                    L"Current algorithm: %s", algoName);
            break;
            
        case ID_TOOL_POLYGON:
            _snwprintf(helpText, 512, 
                    L"How to draw a polygon:\n"
                    L"1. Click for each vertex of the polygon\n"
                    L"2. Right-click to close the polygon\n\n"
                    L"Current algorithm: %s", algoName);
            break;
            
        case ID_TOOL_CLIP:
            _snwprintf(helpText, 512, 
                    L"How to define a clipping window:\n"
                    L"1. Click at the top-left corner\n"
                    L"2. Drag to the bottom-right corner\n"
                    L"3. Release to set the clipping window\n\n"
                    L"Current algorithm: %s", algoName);
            break;
            
        case ID_TOOL_FILL:
            _snwprintf(helpText, 512, 
                    L"How to fill a shape:\n"
                    L"1. Click inside a closed shape\n"
                    L"2. The shape will be filled with the current color\n\n"
                    L"This works for any closed shape (circles, polygons, etc.)");
            break;
            
        default:
            wcscpy(helpText, L"Select a tool to begin drawing");
            break;
    }
    
    SetWindowTextW(g_hHelpText, helpText);
}

void UpdateCardinalControls(HWND hwnd, bool isCardinalSelected) {
    static bool controlsCreated = false;
    
    // Get client area dimensions
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);
    
    int startX = 500; // Position to the right of other controls
    int startY = 20;
    int labelWidth = 80;
    int controlWidth = 120;
    int controlHeight = 24;
    
    if (isCardinalSelected) {
        // Create or show the tension label and dropdown
        if (!controlsCreated) {
            // Create tension label
            CreateWindowW(
                L"STATIC", L"Tension:",
                WS_CHILD | WS_VISIBLE | SS_LEFT,
                startX, startY, labelWidth, controlHeight,
                hwnd, (HMENU)ID_LABEL_THICKNESS + 1, GetModuleHandle(NULL), NULL
            );
            
            // Create tension dropdown
            g_hCardinalTensionCombo = CreateWindowW(
                L"COMBOBOX", L"",
                WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                startX + labelWidth, startY, controlWidth, 200,
                hwnd, (HMENU)ID_COMBO_TENSION, GetModuleHandle(NULL), NULL
            );
            
            // Add tension options
            SendMessage(g_hCardinalTensionCombo, CB_ADDSTRING, 0, (LPARAM)L"0.0");
            SendMessage(g_hCardinalTensionCombo, CB_ADDSTRING, 0, (LPARAM)L"0.25");
            SendMessage(g_hCardinalTensionCombo, CB_ADDSTRING, 0, (LPARAM)L"0.5 (Default)");
            SendMessage(g_hCardinalTensionCombo, CB_ADDSTRING, 0, (LPARAM)L"0.75");
            SendMessage(g_hCardinalTensionCombo, CB_ADDSTRING, 0, (LPARAM)L"1.0");
            
            // Set default selection
            SendMessage(g_hCardinalTensionCombo, CB_SETCURSEL, 2, 0); // 0.5 is default
            
            // Set font
            SendMessage(g_hCardinalTensionCombo, WM_SETFONT, (WPARAM)g_hLabelFont, TRUE);
            
            controlsCreated = true;
        }
        
        // Show the controls
        ShowWindow(g_hCardinalTensionCombo, SW_SHOW);
        ShowWindow(GetDlgItem(hwnd, ID_LABEL_THICKNESS + 1), SW_SHOW);
    } else if (controlsCreated) {
        // Hide the controls when not using Cardinal Spline
        ShowWindow(g_hCardinalTensionCombo, SW_HIDE);
        ShowWindow(GetDlgItem(hwnd, ID_LABEL_THICKNESS + 1), SW_HIDE);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_COMMAND:
            // Handle menu items
            if (HIWORD(wParam) == 0 && lParam == 0) {
                HandleMenuSelection(hwnd, wParam);
                return 0;
            }
            
            // Handle control notifications
            switch (HIWORD(wParam)) {
                case CBN_SELCHANGE:
                    // Handle dropdown selection changes
                    if (LOWORD(wParam) == ID_COMBO_SHAPE) {
                        HandleShapeSelection();
                        
                        // Call UpdateControlVisibility to show/hide the appropriate controls
                        UpdateControlVisibility();
                        return 0;
                    }
                    else if (LOWORD(wParam) == ID_COMBO_ALGORITHM) {
                        HandleAlgorithmSelection();
                        return 0;
                    }
                    else if (LOWORD(wParam) == (ID_COMBO_ALGORITHM + 1)) {  // Fill method dropdown
                        HandleFillMethodSelection();
                        return 0;
                    }
                    else if (LOWORD(wParam) == (ID_COMBO_ALGORITHM + 2)) {  // Quarter selection dropdown
                        HandleQuarterSelection();
                        return 0;
                    }
                    else if (LOWORD(wParam) == ID_POLY_FILL_COMBO) {  // Polygon fill dropdown
                        HandlePolyFillSelection();
                        return 0;
                    }
                    else if (LOWORD(wParam) == ID_COMBO_TENSION) {
                        // Handle tension selection change
                        int index = (int)SendMessage(g_hCardinalTensionCombo, CB_GETCURSEL, 0, 0);
                        if (index != CB_ERR) {
                            switch (index) {
                                case 0: g_cardinalTension = 0.0f; break;
                                case 1: g_cardinalTension = 0.25f; break;
                                case 2: g_cardinalTension = 0.5f; break;
                                case 3: g_cardinalTension = 0.75f; break;
                                case 4: g_cardinalTension = 1.0f; break;
                                default: g_cardinalTension = 0.5f; break;
                            }
                            
                            // Update status message
                            wchar_t msg[256];
                            _snwprintf(msg, 256, L"Cardinal Spline tension set to: %.2f", g_cardinalTension);
                            UpdateStatusBar(msg);
                            
                            // Redraw if we have control points
                            if (!g_polygonPoints.empty()) {
                                InvalidateRect(hwnd, NULL, TRUE);
                            }
                        }
                        return 0;
                    }
                    break;
                    
                case BN_CLICKED:
                    // Handle button clicks
                    if (LOWORD(wParam) == ID_COLOR_BUTTON) {
                        ShowColorDialog(hwnd);
                        return 0;
                    }
                    // Handle the fill checkbox
                    else if (LOWORD(wParam) == ID_FILL_CHECKBOX) {
                        // Update the g_isFilled flag based on checkbox state
                        g_isFilled = (SendMessage(g_hFillCheckbox, BM_GETCHECK, 0, 0) == BST_CHECKED);
                        
                        // Update status bar with clear message
                        wchar_t statusText[256];
                        _snwprintf(statusText, 256, L"Fill option %s - shapes will %sbe filled", 
                                 g_isFilled ? L"enabled" : L"disabled",
                                 g_isFilled ? L"" : L"not ");
                        UpdateStatusBar(statusText);
                        return 0;
                    }
                    break;
            }
            break;
            
        case WM_HSCROLL:
            // Handle trackbar (slider) messages
            if ((HWND)lParam == g_hThicknessSlider) {
                g_lineThickness = (int)SendMessage(g_hThicknessSlider, TBM_GETPOS, 0, 0);
                
                // Update instructions in the status bar
                UpdateInstructions();
                
                return 0;
            }
            break;

        case WM_SIZE:
            {
                // Resize status bar on window resize
                RECT rcClient;
                GetClientRect(hwnd, &rcClient);

                // Resize status bar
                SendMessage(g_hStatusBar, WM_SIZE, 0, 0);
                
                // Move help text window to top right
                if (g_hHelpText) {
                    MoveWindow(g_hHelpText, rcClient.right - 300, 20, 280, 70, TRUE);
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
                RECT rc = GetCanvasRect(hwnd);
                BitBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 
                      g_hdcMem, 0, 0, SRCCOPY);
                
                // If we're collecting points for a curve, draw them
                if (!g_polygonPoints.empty() && g_currentTool == ID_TOOL_CURVE) {
                    // Only show control points while collecting them (before we have 4 points)
                    if (g_polygonPoints.size() < 4) {
                        // Select a pen
                        HPEN hPen = CreatePen(PS_SOLID, g_lineThickness, g_currentColor);
                        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
                        
                        // Draw control points as small circles
                        for (size_t i = 0; i < g_polygonPoints.size(); i++) {
                            // Draw a small circle at each control point
                            Ellipse(hdc, 
                                  g_polygonPoints[i].x + rc.left - 3, 
                                  g_polygonPoints[i].y + rc.top - 3,
                                  g_polygonPoints[i].x + rc.left + 3, 
                                  g_polygonPoints[i].y + rc.top + 3);
                        }
                        
                        // Restore original pen
                        SelectObject(hdc, hOldPen);
                        DeleteObject(hPen);
                    }
                }
                // If we're collecting points for a polygon, draw them
                else if (!g_polygonPoints.empty() && g_currentTool == ID_TOOL_POLYGON) {
                    // Select a pen
                    HPEN hPen = CreatePen(PS_SOLID, g_lineThickness, g_currentColor);
                    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
                    
                    // Draw points
                    for (size_t i = 0; i < g_polygonPoints.size(); i++) {
                        // Adjust for canvas position
                        Ellipse(hdc, 
                              g_polygonPoints[i].x + rc.left - 3, 
                              g_polygonPoints[i].y + rc.top - 3,
                              g_polygonPoints[i].x + rc.left + 3, 
                              g_polygonPoints[i].y + rc.top + 3);
                    }
                    
                    // Draw lines connecting points
                    if (g_polygonPoints.size() > 1) {
                        for (size_t i = 1; i < g_polygonPoints.size(); i++) {
                            MoveToEx(hdc, 
                                  g_polygonPoints[i-1].x + rc.left, 
                                  g_polygonPoints[i-1].y + rc.top, NULL);
                            LineTo(hdc, 
                                 g_polygonPoints[i].x + rc.left, 
                                 g_polygonPoints[i].y + rc.top);
                        }
                    }
                    
                    // Draw line to mouse position if we have at least one point
                    if (g_polygonPoints.size() > 0) {
                        POINT pt;
                        GetCursorPos(&pt);
                        ScreenToClient(hwnd, &pt);
                        
                        if (PtInRect(&rc, pt)) {
                            // Draw line from last point to cursor
                            MoveToEx(hdc, 
                                  g_polygonPoints.back().x + rc.left, 
                                  g_polygonPoints.back().y + rc.top, NULL);
                            LineTo(hdc, pt.x, pt.y);
                        }
                    }
                    
                    // Restore original pen
                    SelectObject(hdc, hOldPen);
                    DeleteObject(hPen);
                }
                
                EndPaint(hwnd, &ps);
                return 0;
            }

        case WM_LBUTTONDOWN:
            {
                int x = GET_X_LPARAM(lParam);
                int y = GET_Y_LPARAM(lParam);
                
                // Check if the click is within the canvas area
                if (x >= g_canvasRect.left && x <= g_canvasRect.right &&
                    y >= g_canvasRect.top && y <= g_canvasRect.bottom) {
                    
                    // Convert to canvas coordinates
                    x = x - g_canvasRect.left;
                    y = y - g_canvasRect.top;
                    
                    // Handle different tools
                    if (g_currentTool == ID_TOOL_POLYGON) {
                        // Make sure polygon fill options are visible
                        UpdateControlVisibility();
                        
                        // Store current fill checkbox state before drawing
                        bool currentFillState = g_isFilled;
                        
                        // Add point to polygon
                        POINT pt = {x, y};
                        g_polygonPoints.push_back(pt);
                        
                        // Draw a small point to mark the vertex - always fill with a small dot regardless of fill state
                        DrawCircle(g_hdcMem, x, y, 2, g_currentColor, 1, true);
                        
                        // Restore fill state
                        g_isFilled = currentFillState;
                        SendMessage(g_hFillCheckbox, BM_SETCHECK, g_isFilled ? BST_CHECKED : BST_UNCHECKED, 0);
                        
                        // If there are at least 2 points, draw a line between the last two
                        if (g_polygonPoints.size() >= 2) {
                            POINT p1 = g_polygonPoints[g_polygonPoints.size() - 2];
                            POINT p2 = g_polygonPoints[g_polygonPoints.size() - 1];
                            DrawLine(g_hdcMem, p1.x, p1.y, p2.x, p2.y, g_currentColor, g_lineThickness);
                        }
                        
                        // Invalidate the canvas area to redraw
                        InvalidateRect(hwnd, &g_canvasRect, FALSE);
                    }
                    else if (g_currentTool == ID_TOOL_CURVE) {
                        // Add point for curve
                        POINT pt = {x, y};
                        
                        // Only add if we have less than 4 points
                        if (g_polygonPoints.size() < 4) {
                            g_polygonPoints.push_back(pt);
                            
                            // Draw a visible control point marker
                            DrawCircle(g_hdcMem, x, y, 3, g_currentColor, 1, true);
                            
                            // Show status message with point count
                            wchar_t msg[256];
                            _snwprintf(msg, 256, L"Cardinal Spline: Point %d of 4 collected.", (int)g_polygonPoints.size());
                            UpdateStatusBar(msg);
                            
                            // Force redraw to show the point
                            InvalidateRect(hwnd, NULL, TRUE);
                            
                            // If we have collected 4 points, draw the curve
                            if (g_polygonPoints.size() == 4) {
                                // Draw the curve based on selected algorithm
                                if (g_currentAlgorithm == ID_ALGO_CARDINAL || 
                                    g_currentAlgorithm == ID_ALGO_HERMITE) {
                                    
                                    // Clear the canvas to remove the control points
                                    ClearCanvas(g_hdcMem);
                                    
                                    // Draw the Cardinal Spline with the proper algorithm
                                    DrawCardinalSpline(g_hdcMem, g_polygonPoints, g_currentColor, g_lineThickness);
                                    
                                    // Update status message
                                    wchar_t msg[256];
                                    _snwprintf(msg, 256, L"Cardinal Spline drawn with 4 points. Click to start a new curve.");
                                    UpdateStatusBar(msg);
                                    
                                    // Force redraw
                                    InvalidateRect(hwnd, NULL, TRUE);
                                    
                                    // Clear points for next curve
                                    g_polygonPoints.clear();
                                }
                            }
                        } else {
                            // Reset points for a new curve if user clicks after completing a curve
                            g_polygonPoints.clear();
                            g_polygonPoints.push_back(pt);
                            
                            // Update status
                            wchar_t msg[256];
                            _snwprintf(msg, 256, L"Cardinal Spline: Point 1 of 4 collected.");
                            UpdateStatusBar(msg);
                            
                            // Redraw
                            InvalidateRect(hwnd, NULL, TRUE);
                        }
                    }
                    else if (g_currentTool == ID_TOOL_ELLIPSE) {
                        // Set ellipse drawing mode flag
                        g_ellipseDrawingMode = true;
                        
                        // Handle 3-point ellipse drawing
                        if (g_ellipseClickCount == 0) {
                            // First click - store the center point
                            g_ellipseCenter.x = x;
                            g_ellipseCenter.y = y;
                            g_ellipseClickCount++;
                            
                            // Draw a small point to mark the center
                            DrawCircle(g_hdcMem, x, y, 2, g_currentColor, 1, true);
                            InvalidateRect(hwnd, &g_canvasRect, FALSE);
                            
                            // Update status bar with next instruction
                            UpdateEllipseStatus();
                            
                            // Make sure we're not in normal drawing mode to prevent the diagonal line
                            g_isDrawing = false;
                        }
                        else if (g_ellipseClickCount == 1) {
                            // Second click - store first axis point
                            g_ellipsePoint1.x = x;
                            g_ellipsePoint1.y = y;
                            g_ellipseClickCount++;
                            
                            // Draw a small point
                            DrawCircle(g_hdcMem, x, y, 2, g_currentColor, 1, true);
                            InvalidateRect(hwnd, &g_canvasRect, FALSE);
                            
                            // Update status bar with next instruction
                            UpdateEllipseStatus();
                        }
                        else if (g_ellipseClickCount == 2) {
                            // Third click - store second axis point
                            g_ellipsePoint2.x = x;
                            g_ellipsePoint2.y = y;
                            
                            // Calculate a and b for the ellipse
                            int dx1 = g_ellipsePoint1.x - g_ellipseCenter.x;
                            int dx2 = g_ellipsePoint2.x - g_ellipseCenter.x;
                            int dy1 = g_ellipsePoint1.y - g_ellipseCenter.y;
                            int dy2 = g_ellipsePoint2.y - g_ellipseCenter.y;
                            
                            g_ellipseA = (dx1 < 0) ? -dx1 : dx1;  // abs(dx1)
                            int absX2 = (dx2 < 0) ? -dx2 : dx2;   // abs(dx2)
                            g_ellipseA = (g_ellipseA > absX2) ? g_ellipseA : absX2;  // max of the two
                            
                            g_ellipseB = (dy1 < 0) ? -dy1 : dy1;  // abs(dy1)
                            int absY2 = (dy2 < 0) ? -dy2 : dy2;   // abs(dy2)
                            g_ellipseB = (g_ellipseB > absY2) ? g_ellipseB : absY2;  // max of the two
                            
                            // Clear the previous drawing to remove the dots and guide lines
                            ClearCanvas(g_hdcMem);
                            
                            // Make sure we have reasonable values
                            if (g_ellipseA <= 0) g_ellipseA = 1;
                            if (g_ellipseB <= 0) g_ellipseB = 1;
                            
                            try {
                                // Draw the ellipse using the selected algorithm
                                switch (g_currentAlgorithm) {
                                    case ID_ALGO_MIDPOINT:
                                        // Draw ellipse using midpoint algorithm
                                        DrawEllipse(g_hdcMem, g_ellipseCenter.x, g_ellipseCenter.y, g_ellipseA, g_ellipseB, 
                                                 g_currentColor, g_lineThickness, g_isFilled);
                                        break;
                                    case ID_ALGO_DIRECT:
                                        // Draw ellipse using direct algorithm
                                        DrawEllipse(g_hdcMem, g_ellipseCenter.x, g_ellipseCenter.y, g_ellipseA, g_ellipseB, 
                                                  g_currentColor, g_lineThickness, g_isFilled);
                                        break;
                                    case ID_ALGO_POLAR:
                                        // Draw ellipse using polar algorithm
                                        DrawEllipse(g_hdcMem, g_ellipseCenter.x, g_ellipseCenter.y, g_ellipseA, g_ellipseB, 
                                                  g_currentColor, g_lineThickness, g_isFilled);
                                        break;
                                    default:
                                        // Default ellipse drawing
                                        DrawEllipse(g_hdcMem, g_ellipseCenter.x, g_ellipseCenter.y, g_ellipseA, g_ellipseB, 
                                                  g_currentColor, g_lineThickness, g_isFilled);
                                        break;
                                }
                            }
                            catch (...) {
                                // If drawing with fill fails, try again with just the outline
                                DrawEllipse(g_hdcMem, g_ellipseCenter.x, g_ellipseCenter.y, g_ellipseA, g_ellipseB, 
                                         g_currentColor, g_lineThickness, false);
                                
                                // Inform the user of the issue
                                MessageBoxW(g_hwnd, L"Filling operation failed. Drawing outline only.", 
                                         L"Fill Error", MB_OK | MB_ICONWARNING);
                            }
                            
                            // Reset click counter for next ellipse
                            g_ellipseClickCount = 0;

                            // Reset all ellipse points to avoid interference with the next ellipse
                            g_ellipseCenter.x = 0;
                            g_ellipseCenter.y = 0;
                            g_ellipsePoint1.x = 0;
                            g_ellipsePoint1.y = 0;
                            g_ellipsePoint2.x = 0;
                            g_ellipsePoint2.y = 0;
                            g_ellipseA = 0;
                            g_ellipseB = 0;
                            
                            // Make sure we're not in drawing mode to prevent guide lines
                            g_isDrawing = false;
                            
                            // Invalidate the canvas area to redraw
                            InvalidateRect(hwnd, &g_canvasRect, FALSE);
                            
                            // Restore normal instructions
                            UpdateInstructions();
                        }
                    }
                    else if (g_currentTool == ID_TOOL_FILL) {
                        // Get the target color (the color of the pixel at the click position)
                        COLORREF targetColor = GetPixel(g_hdcMem, x, y);
                        
                        // Don't fill if clicking on the fill color already
                        if (targetColor == g_currentColor) {
                            MessageBoxW(hwnd, L"This area is already filled with the current color.", 
                                       L"Fill Tool", MB_OK | MB_ICONINFORMATION);
                            return 0;
                        }
                        
                        // Use the safer queue-based flood fill
                        myFloodFillQueue(g_hdcMem, x, y, targetColor, g_currentColor);
                        
                        // Update the canvas
                        InvalidateRect(hwnd, &g_canvasRect, FALSE);
                    }
                    else {
                        g_startPoint.x = x;
                        g_startPoint.y = y;
                        g_endPoint = g_startPoint;
                        g_isDrawing = true;
                    }
                }
            }
            return 0;

        case WM_MOUSEMOVE:
            {
                // Get current mouse position
                int x = GET_X_LPARAM(lParam);
                int y = GET_Y_LPARAM(lParam);
                
                // Show coordinates in status bar
                wchar_t msg[100];
                _snwprintf(msg, 100, L"X: %d, Y: %d", x, y);
                SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)msg);
                
                // Update end point for drag operations
                if (g_isDrawing) {
                    g_endPoint.x = x - g_canvasRect.left;
                    g_endPoint.y = y - g_canvasRect.top;
                    
                    // Force redraw to update guidelines
                    InvalidateRect(hwnd, NULL, TRUE);
                }
                
                // For ellipse 3-point drawing
                if (g_ellipseDrawingMode && g_ellipseClickCount > 0) {
                    InvalidateRect(hwnd, NULL, TRUE);
                }
                
                // For polygon and curve drawing - update preview
                if (!g_polygonPoints.empty() && (g_currentTool == ID_TOOL_POLYGON || g_currentTool == ID_TOOL_CURVE)) {
                    InvalidateRect(hwnd, NULL, TRUE);
                }
                
                // Update status bar with coordinates
                wchar_t coordText[64];
                _snwprintf(coordText, 64, L"X: %d, Y: %d", x, y);
                SendMessage(g_hStatusBar, SB_SETTEXT, 1, (LPARAM)coordText);
                
                return 0;
            }

        case WM_LBUTTONUP:
            {
                if (g_isDrawing) {
                    int x = GET_X_LPARAM(lParam);
                    int y = GET_Y_LPARAM(lParam);
                    
                    // Convert to canvas coordinates
                    g_endPoint.x = x - g_canvasRect.left;
                    g_endPoint.y = y - g_canvasRect.top;
                    
                    // Draw the shape permanently
                    switch (g_currentTool) {
                        case ID_TOOL_LINE:
                            // For lines, we should completely ignore fill settings since lines can't be filled
                            try {
                                // Create a line object
                                Line line(g_hdcMem);
                                
                                // Use the appropriate algorithm based on current selection
                                switch (g_currentAlgorithm) {
                                    case ID_ALGO_DDA:
                                        line.DrawLineDDA(g_startPoint.x, g_startPoint.y, 
                                                     g_endPoint.x, g_endPoint.y, g_currentColor);
                                        break;
                                    
                                    case ID_ALGO_MIDPOINT:
                                        line.DrawLineMidpoint(g_startPoint.x, g_startPoint.y, 
                                                          g_endPoint.x, g_endPoint.y, g_currentColor);
                                        break;
                                    
                                    case ID_ALGO_PARAMETRIC:
                                        line.DrawLineParametric(g_startPoint.x, g_startPoint.y, 
                                                           g_endPoint.x, g_endPoint.y, g_currentColor);
                                        break;
                                    
                                    default:
                                        // Fallback to DDA
                                        line.DrawLineDDA(g_startPoint.x, g_startPoint.y, 
                                                     g_endPoint.x, g_endPoint.y, g_currentColor);
                                        break;
                                }
                                
                                // If line thickness > 1, draw a thicker line using GDI
                                if (g_lineThickness > 1) {
                                    HPEN hPen = CreatePen(PS_SOLID, g_lineThickness, g_currentColor);
                                    HPEN hOldPen = (HPEN)SelectObject(g_hdcMem, hPen);
                                    
                                    MoveToEx(g_hdcMem, g_startPoint.x, g_startPoint.y, NULL);
                                    LineTo(g_hdcMem, g_endPoint.x, g_endPoint.y);
                                    
                                    SelectObject(g_hdcMem, hOldPen);
                                    DeleteObject(hPen);
                                }
                            }
                            catch (...) {
                                // Fallback to simple GDI line if custom implementation fails
                                DrawLine(g_hdcMem, g_startPoint.x, g_startPoint.y, 
                                       g_endPoint.x, g_endPoint.y, g_currentColor, g_lineThickness);
                            }
                            break;
                            
                        case ID_TOOL_CIRCLE:
                            {
                                // Calculate radius
                                int dx = g_endPoint.x - g_startPoint.x;
                                int dy = g_endPoint.y - g_startPoint.y;
                                int radius = (int)sqrt((double)(dx*dx + dy*dy));
                                
                                // Safety check - prevent negative or zero radius
                                if (radius <= 0) radius = 1;
                                
                                try {
                                    // Draw the circle with or without fill based on radio button selection
                                    DrawCircle(g_hdcMem, g_startPoint.x, g_startPoint.y, radius, 
                                             g_currentColor, g_lineThickness, g_isFilled);
                                }
                                catch (...) {
                                    // Fallback to simple circle if custom implementation fails
                                    DrawCircle(g_hdcMem, g_startPoint.x, g_startPoint.y, radius, 
                                             g_currentColor, g_lineThickness, false);
                                }
                            }
                            break;
                            
                            
                        // Note: Ellipse is now handled by the 3-point method in WM_LBUTTONDOWN
                        
                        case ID_TOOL_CURVE:
                            // For now, just draw a line
                            DrawLine(g_hdcMem, g_startPoint.x, g_startPoint.y, g_endPoint.x, g_endPoint.y, g_currentColor, g_lineThickness);
                            break;
                            
                        case ID_TOOL_CLIP:
                            // For now, just draw a rectangle
                            HPEN hPen = CreatePen(PS_DASH, 1, g_currentColor);
                            HPEN hOldPen = (HPEN)SelectObject(g_hdcMem, hPen);
                            HBRUSH hOldBrush = (HBRUSH)SelectObject(g_hdcMem, GetStockObject(NULL_BRUSH));
                            
                            Rectangle(g_hdcMem, g_startPoint.x, g_startPoint.y, g_endPoint.x, g_endPoint.y);
                            
                            SelectObject(g_hdcMem, hOldPen);
                            SelectObject(g_hdcMem, hOldBrush);
                            DeleteObject(hPen);
                            break;
                    }
                    
                    g_isDrawing = false;
                    
                    // Redraw the canvas
                    InvalidateRect(hwnd, &g_canvasRect, FALSE);
                    
                    // Ensure the fill checkbox state is synchronized with g_isFilled
                    SendMessage(g_hFillCheckbox, BM_SETCHECK, g_isFilled ? BST_CHECKED : BST_UNCHECKED, 0);
                }
            }
            return 0;

        case WM_RBUTTONDOWN:
            {
                // Right-click to finish polygon
                if (g_currentTool == ID_TOOL_POLYGON && g_polygonPoints.size() >= 3) {
                    // Close the polygon by connecting the last point to the first
                    POINT first = g_polygonPoints[0];
                    POINT last = g_polygonPoints[g_polygonPoints.size() - 1];
                    DrawLine(g_hdcMem, last.x, last.y, first.x, first.y, g_currentColor, g_lineThickness);
                    
                    // If fill is enabled, fill the polygon
                    if (g_isFilled) {
                        // Use the selected fill method
                        switch(g_polyFillMethod) {
                            case 0: // Convex Fill
                                {
                                    // Convert to point array for convex fill
                                    point* convexPoints = new point[g_polygonPoints.size()];
                                    for (size_t i = 0; i < g_polygonPoints.size(); i++) {
                                        convexPoints[i].x = g_polygonPoints[i].x;
                                        convexPoints[i].y = g_polygonPoints[i].y;
                                    }
                                    
                                    try {
                                        // Check if the polygon is actually convex
                                        std::vector<point> pointsVec(convexPoints, convexPoints + g_polygonPoints.size());
                                        if (IsPolygonConvex(pointsVec)) {
                                            convexfill(g_hdcMem, convexPoints, g_polygonPoints.size(), g_currentColor);
                                        } else {
                                            // If not convex, fall back to general fill
                                            MessageBoxW(hwnd, L"Polygon is not convex. Using general fill method.", 
                                                    L"Fill Method", MB_OK | MB_ICONINFORMATION);
                                            fillGeneralPolygon(g_hdcMem, convexPoints, g_polygonPoints.size(), g_currentColor);
                                        }
                                    }
                                    catch(...) {
                                        MessageBoxW(hwnd, L"Failed to fill polygon. Drawing outline only.", 
                                                L"Fill Error", MB_OK | MB_ICONWARNING);
                                    }
                                    
                                    // Clean up
                                    delete[] convexPoints;
                                }
                                break;
                                
                            case 1: // General Fill
                                {
                                    // Convert to point array for general fill
                                    point* generalPoints = new point[g_polygonPoints.size()];
                                    for (size_t i = 0; i < g_polygonPoints.size(); i++) {
                                        generalPoints[i].x = g_polygonPoints[i].x;
                                        generalPoints[i].y = g_polygonPoints[i].y;
                                    }
                                    
                                    try {
                                        fillGeneralPolygon(g_hdcMem, generalPoints, g_polygonPoints.size(), g_currentColor);
                                    }
                                    catch(...) {
                                        MessageBoxW(hwnd, L"Failed to fill polygon. Drawing outline only.", 
                                                L"Fill Error", MB_OK | MB_ICONWARNING);
                                    }
                                    
                                    // Clean up
                                    delete[] generalPoints;
                                }
                                break;
                                
                            case 2: // Flood Fill
                                {
                                    // Calculate centroid of polygon for flood fill starting point
                                    int centerX = 0, centerY = 0;
                                    for (size_t i = 0; i < g_polygonPoints.size(); i++) {
                                        centerX += g_polygonPoints[i].x;
                                        centerY += g_polygonPoints[i].y;
                                    }
                                    centerX /= g_polygonPoints.size();
                                    centerY /= g_polygonPoints.size();
                                    
                                    // Get the background color
                                    COLORREF bgColor = GetPixel(g_hdcMem, centerX, centerY);
                                    
                                    // Use the queue-based flood fill for safety
                                    try {
                                        myFloodFillQueue(g_hdcMem, centerX, centerY, bgColor, g_currentColor);
                                    }
                                    catch(...) {
                                        MessageBoxW(hwnd, L"Flood fill failed. Try another fill method.", 
                                                L"Fill Error", MB_OK | MB_ICONWARNING);
                                    }
                                }
                                break;
                                
                            default: // Default to standard GDI fill
                                {
                                    // Convert the POINT array to point array for standard GDI polygon
                                    POINT* polyPoints = new POINT[g_polygonPoints.size()];
                                    for (size_t i = 0; i < g_polygonPoints.size(); i++) {
                                        polyPoints[i].x = g_polygonPoints[i].x;
                                        polyPoints[i].y = g_polygonPoints[i].y;
                                    }
                                    
                                    try {
                                        // Use standard GDI Polygon function for simplicity and reliability
                                        HBRUSH hBrush = CreateSolidBrush(g_currentColor);
                                        HBRUSH hOldBrush = (HBRUSH)SelectObject(g_hdcMem, hBrush);
                                        
                                        // Use the standard GDI Polygon function
                                        Polygon(g_hdcMem, polyPoints, (int)g_polygonPoints.size());
                                        
                                        SelectObject(g_hdcMem, hOldBrush);
                                        DeleteObject(hBrush);
                                    } 
                                    catch (...) {
                                        MessageBoxW(hwnd, L"Failed to fill polygon. Drawing outline only.", 
                                                L"Fill Error", MB_OK | MB_ICONWARNING);
                                    }
                                    
                                    // Clean up
                                    delete[] polyPoints;
                                }
                        }
                    }
                    
                    // Show status message
                    wchar_t msg[256];
                    _snwprintf(msg, 256, L"Polygon completed with %d points.", (int)g_polygonPoints.size());
                    UpdateStatusBar(msg);
                    
                    // Clear the polygon points
                    g_polygonPoints.clear();
                    
                    // Redraw the canvas
                    InvalidateRect(hwnd, &g_canvasRect, FALSE);
                }
            }
            return 0;

        case WM_ERASEBKGND:
            // To avoid flicker, handle all drawing in WM_PAINT
            return 1;

        // Handle double-click to finish a Cardinal Spline
        case WM_LBUTTONDBLCLK:
            {
                // Handle curve completion on double-click
                if (g_currentTool == ID_TOOL_CURVE && g_polygonPoints.size() >= 4) {
                    // Get coordinates
                    int x = GET_X_LPARAM(lParam);
                    int y = GET_Y_LPARAM(lParam);
                    
                    // Only process if inside canvas
                    if (PtInRect(&g_canvasRect, POINT{ x, y })) {
                        // Add final point if it's different from the last one
                        if (g_polygonPoints.back().x != x || g_polygonPoints.back().y != y) {
                            g_polygonPoints.push_back(POINT{ x, y });
                        }
                        
                        // Adjust coordinates to be relative to the canvas
                        std::vector<POINT> canvasPoints = g_polygonPoints;
                        for (auto& pt : canvasPoints) {
                            pt.x -= g_canvasRect.left;
                            pt.y -= g_canvasRect.top;
                        }
                        
                        // Draw the final curve
                        HDC hdcMem = g_hdcMem;
                        
                        // Draw Cardinal Spline
                        if (canvasPoints.size() >= 4) {
                            // Draw Cardinal Spline to the memory DC
                            DrawCardinalSpline(hdcMem, canvasPoints, g_currentColor, g_lineThickness);
                        }
                        
                        // Update status message
                        wchar_t msg[256];
                        _snwprintf(msg, 256, L"Curve completed with %d control points.", g_polygonPoints.size());
                        UpdateStatusBar(msg);
                        
                        // Clear control points for next curve
                        g_polygonPoints.clear();
                        
                        // Force window redraw
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                    
                    return 0;
                }
            }
            break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Helper function to create custom fonts
HFONT CreateCustomFont(int size, bool bold) {
    return CreateFontW(
        size, 0, 0, 0, 
        bold ? FW_BOLD : FW_NORMAL,
        FALSE, FALSE, FALSE, 
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"Segoe UI"
    );
}

// Add the DrawCardinalSpline function
void DrawCardinalSpline(HDC hdc, const std::vector<POINT>& points, COLORREF color, int thickness) {
    if (points.size() < 4) return; // Need at least 4 points for Cardinal spline
    
    // Convert POINT array to Point struct array
    std::vector<Point> controlPoints;
    for (const auto& pt : points) {
        controlPoints.push_back({ (double)pt.x, (double)pt.y });
    }
    
    // Use the tension parameter to calculate c
    double c = (1.0 - g_cardinalTension) / 2.0;
    
    // Draw the Cardinal spline using Hermite approach
    Derivative t1 = { c * (controlPoints[2].x - controlPoints[0].x), 
                      c * (controlPoints[2].y - controlPoints[0].y) };
                      
    for (size_t i = 2; i < controlPoints.size() - 1; ++i) {
        Derivative t2 = { c * (controlPoints[i+1].x - controlPoints[i-1].x), 
                          c * (controlPoints[i+1].y - controlPoints[i-1].y) };
                          
        DrawHermite(hdc, controlPoints[i-1], controlPoints[i], t1, t2, color, thickness);
        t1 = t2;
    }
}

// Line drawing using DDA algorithm

// Polygon filling structures and algorithms

// Global counter for recursive calls to prevent stack overflow
int g_recursionDepth = 0;
const int MAX_RECURSION_DEPTH = 1000; // Limit to prevent stack overflow

// Flood fill implementation (recursive)
void myFloodFill(HDC hdc, int x, int y, COLORREF bc, COLORREF fc) {
    // Initialize counter on first call
    if (g_recursionDepth == 0) {
        g_recursionDepth = 0;
    }
    
    // Prevent stack overflow with recursion limit
    if (g_recursionDepth >= MAX_RECURSION_DEPTH) {
        // Show a message only on the first overflow detection
        if (g_recursionDepth == MAX_RECURSION_DEPTH) {
            MessageBoxW(NULL, L"Recursive fill depth limit reached. Switching to queue-based fill.", 
                      L"Fill Limit", MB_OK | MB_ICONINFORMATION);
            
            // Reset counter for next time
            g_recursionDepth = 0;
            
            // Use the queue-based fill as fallback
            myFloodFillQueue(hdc, x, y, bc, fc);
        }
        return;
    }
    
    // Bounds checking
    if (x < 0 || y < 0 || x >= 2000 || y >= 2000)
        return;
    
    // Safely get the pixel color
    COLORREF current;
    try {
        current = GetPixel(hdc, x, y);
    }
    catch (...) {
        g_recursionDepth = 0; // Reset counter on error
        return;
    }
    
    // Stop if we hit the boundary color or already filled color
    if (current == bc || current == fc) {
        return;
    }
    
    try {
        // Fill current pixel
        SetPixel(hdc, x, y, fc);
        
        // Increment recursion depth
        g_recursionDepth++;
        
        // Recursively fill the 4-connected neighbors
        myFloodFill(hdc, x + 1, y, bc, fc);
        myFloodFill(hdc, x - 1, y, bc, fc);
        myFloodFill(hdc, x, y + 1, bc, fc);
        myFloodFill(hdc, x, y - 1, bc, fc);
        
        // Decrement recursion depth
        g_recursionDepth--;
    }
    catch (...) {
        // Reset counter and switch to queue-based on exception
        g_recursionDepth = 0;
        myFloodFillQueue(hdc, x, y, bc, fc);
    }
    
    // Reset counter when the initial call completes
    if (g_recursionDepth == 0) {
        g_recursionDepth = 0;
    }
}

// Flood fill implementation using queue (non-recursive, prevents stack overflow)
void myFloodFillQueue(HDC hdc, int x, int y, COLORREF bc, COLORREF fc) {
    try {
        // Bounds check on starting point
        if (x < 0 || y < 0 || x >= 5000 || y >= 5000) return;
        
        // If already filled or boundary color, do nothing
        COLORREF targetColor = GetPixel(hdc, x, y);
        if (targetColor == bc || targetColor == fc)
            return;
        
        // Use a queue to store pixels to fill
        std::queue<point> q;
        q.push(point(x, y));
        
        // Safety counter to prevent infinite loops
        int processedPixels = 0;
        const int MAX_PIXELS = 500000; // Limit the number of pixels to fill
        
        // Create a 2D array to track visited pixels
        // Using a hash set would be better but this is simpler for now
        bool* visited = new bool[2000 * 2000];
        memset(visited, 0, 2000 * 2000 * sizeof(bool));
        
        while (!q.empty() && processedPixels < MAX_PIXELS) {
            point p = q.front();
            q.pop();
            
            int px = (int)p.x;
            int py = (int)p.y;
            
            // Skip if out of bounds
            if (px < 0 || py < 0 || px >= 2000 || py >= 2000)
                continue;
                
            // Skip if already visited
            if (visited[py * 2000 + px])
                continue;
                
            visited[py * 2000 + px] = true;
            processedPixels++;
            
            // Check if pixel is within the desired area
            COLORREF currentColor = GetPixel(hdc, px, py);
            if (currentColor == bc || currentColor == fc)
                continue;
            
            // Fill this pixel
            SetPixel(hdc, px, py, fc);
            
            // Add the 4-connected neighbors to queue
            if (px + 1 < 2000) q.push(point(px + 1, py));
            if (px - 1 >= 0) q.push(point(px - 1, py));
            if (py + 1 < 2000) q.push(point(px, py + 1));
            if (py - 1 >= 0) q.push(point(px, py - 1));
        }
        
        // Clean up
        delete[] visited;
        
        // If we hit the limit, show a message
        if (processedPixels >= MAX_PIXELS) {
            MessageBoxW(NULL, L"Flood fill area too large - fill operation limited", 
                       L"Fill Limit Reached", MB_OK | MB_ICONINFORMATION);
        }
    }
    catch (...) {
        // Catch all exceptions to prevent crashes
    }
}

// Add a simplified placeholder for fillGeneralPolygon before myFloodFill
// This function implements a general scan-line filling algorithm for any polygon
void fillGeneralPolygon(HDC hdc, point p[], int n, COLORREF c) {
    // Check for valid polygon
    if (n < 3) return;

    try {
        // Find the min and max y-values to determine scan-line range
        int minY = INT_MAX;
        int maxY = INT_MIN;
        for (int i = 0; i < n; i++) {
            if ((int)p[i].y < minY) minY = (int)p[i].y;
            if ((int)p[i].y > maxY) maxY = (int)p[i].y;
        }

        // Strict bounds checking to prevent array out-of-bounds
        minY = std::max(0, minY);
        maxY = std::min(2000, maxY);  // Safe upper limit below the 3000 edge table size

        // For each scan line
        for (int y = minY; y <= maxY; y++) {
            // Find all intersections with this scan line
            std::vector<int> intersections;
            intersections.reserve(n); // Reserve space to avoid reallocations
            
            for (int i = 0; i < n; i++) {
                // Get the current edge (p[i] to p[i+1])
                int next = (i + 1) % n;
                double y1 = p[i].y;
                double y2 = p[next].y;
                
                // Skip horizontal edges
                if (fabs(y1 - y2) < 0.001) continue;

                // Check if the scan line intersects this edge
                if ((y >= y1 && y < y2) || (y >= y2 && y < y1)) {
                    double x1 = p[i].x;
                    double x2 = p[next].x;
                    
                    // Calculate x-coordinate of intersection
                    // Using the line equation: x = x1 + (y - y1) * (x2 - x1) / (y2 - y1)
                    double x = x1 + (y - y1) * (x2 - x1) / (y2 - y1);
                    
                    // Bound check x coordinate to prevent extreme values
                    if (x > -10000 && x < 10000) {
                        intersections.push_back((int)x);
                    }
                }
            }

            // Sort intersections by x-coordinate
            if (!intersections.empty()) {
                std::sort(intersections.begin(), intersections.end());

                // Fill between pairs of intersections
                for (size_t i = 0; i < intersections.size() - 1; i += 2) {
                    if (i + 1 < intersections.size()) {
                        int left = std::max(0, intersections[i]);
                        int right = std::min(5000, intersections[i + 1]);
                        
                        if (left < right) {
                            // Draw the horizontal span
                            DrawLineDDA(hdc, left, y, right, y, c);
                        }
                    }
                }
            }
        }
    }
    catch (...) {
        // Handle any exceptions that might occur during filling
        // Just silently catch them to prevent crashes
    }
}

// Add a function to check if a polygon is convex before the flood fill functions
// Returns true if the polygon is convex (has at most 2 intersections with any horizontal line)
bool IsPolygonConvex(const std::vector<point>& points) {
    if (points.size() < 3) return false;
    
    // A convex polygon has all interior angles less than 180 degrees
    // We can check this by verifying that all cross products have the same sign
    
    bool hasPositive = false;
    bool hasNegative = false;
    
    int n = (int)points.size();
    for (int i = 0; i < n; i++) {
        int j = (i + 1) % n;
        int k = (i + 2) % n;
        
        // Calculate vectors for consecutive edges
        double dx1 = points[j].x - points[i].x;
        double dy1 = points[j].y - points[i].y;
        double dx2 = points[k].x - points[j].x;
        double dy2 = points[k].y - points[j].y;
        
        // Calculate cross product (z-component in 2D space)
        double crossProduct = dx1 * dy2 - dy1 * dx2;
        
        // Check sign
        if (crossProduct > 0) hasPositive = true;
        if (crossProduct < 0) hasNegative = true;
        
        // If we have both positive and negative cross products, the polygon is not convex
        if (hasPositive && hasNegative) return false;
    }
    
    return true;
}

// Add a new function to fill a circle
void FillCircle(HDC hdc, int centerX, int centerY, int radius, COLORREF color) {
    // Safety check - ensure radius is positive
    if (radius <= 0) {
        radius = 1;  // Minimum radius
    }
    
    // Use standard GDI fill for simplicity and reliability
    StandardFill(hdc, centerX, centerY, radius, color);
}

// Helper function for standard circle fill using GDI
void StandardFill(HDC hdc, int centerX, int centerY, int radius, COLORREF color) {
    // Create a solid brush with the specified color
    HBRUSH hBrush = CreateSolidBrush(color);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
    
    // Create a pen with the same color for a smooth edge
    HPEN hPen = CreatePen(PS_SOLID, 1, color);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    
    // Use GDI Ellipse function to draw a filled circle
    Ellipse(hdc, centerX - radius, centerY - radius, centerX + radius, centerY + radius);
    
    // Clean up
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hBrush);
    DeleteObject(hPen);
}

// Update quarter selection controls visibility based on current tool
void UpdateQuarterSelectionVisibility() {
    // Show quarter selection dropdown only when Circle tool is selected
    if (g_currentTool == ID_TOOL_CIRCLE) {
        ShowWindow(g_hQuarterLabel, SW_SHOW);
        ShowWindow(g_hQuarterCombo, SW_SHOW);
    } else {
        // Hide it for other tools
        ShowWindow(g_hQuarterLabel, SW_HIDE);
        ShowWindow(g_hQuarterCombo, SW_HIDE);
    }
}

#define ID_RADIO_FILL_NONE 9001  // Change to a more reliable value
#define ID_RADIO_FILL_SOLID 9002

// Make sure these are all defined before the global variables
HWND g_hRadioFillNone;
HWND g_hRadioFillSolid;

// Handle polygon fill method selection
void HandlePolyFillSelection() {
    // Get the current selection
    int selection = (int)SendMessage(g_hPolyFillCombo, CB_GETCURSEL, 0, 0);
    
    // Update the global variable
    g_polyFillMethod = selection;
    
    // Update status bar with selected fill method
    wchar_t statusText[256];
    const wchar_t* methodNames[] = {L"Convex Fill", L"General Fill", L"Flood Fill"};
    if (selection >= 0 && selection < 3) {
        _snwprintf(statusText, 256, L"Polygon fill method set to: %s", methodNames[selection]);
        UpdateStatusBar(statusText);
    }
}

// Update control visibility based on current tool
void UpdateControlVisibility() {
    // Show quarter selection dropdown only when Circle tool is selected
    if (g_currentTool == ID_TOOL_CIRCLE) {
        ShowWindow(g_hQuarterLabel, SW_SHOW);
        ShowWindow(g_hQuarterCombo, SW_SHOW);
    } else {
        // Hide it for other tools
        ShowWindow(g_hQuarterLabel, SW_HIDE);
        ShowWindow(g_hQuarterCombo, SW_HIDE);
    }
    
    // Show polygon fill options only when Polygon tool is selected
    if (g_currentTool == ID_TOOL_POLYGON) {
        ShowWindow(g_hPolyFillLabel, SW_SHOW);
        ShowWindow(g_hPolyFillCombo, SW_SHOW);
    } else {
        // Hide for other tools
        ShowWindow(g_hPolyFillLabel, SW_HIDE);
        ShowWindow(g_hPolyFillCombo, SW_HIDE);
    }
}

 
