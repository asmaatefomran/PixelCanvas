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
#include <algorithm> // For std::max

// Include the common controls library (Visual Studio only)
#if defined(_MSC_VER)
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")
#endif

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

// Drawing tools
#define ID_TOOL_LINE 2001
#define ID_TOOL_CIRCLE 2002
#define ID_TOOL_ELLIPSE 2003
#define ID_TOOL_CURVE 2004
#define ID_TOOL_POLYGON 2005
#define ID_TOOL_CLIP 2006

// Algorithms
#define ID_ALGO_DDA 3001
#define ID_ALGO_MIDPOINT 3002
#define ID_ALGO_PARAMETRIC 3003
#define ID_ALGO_POLAR 3004
#define ID_ALGO_BEZIER 3005
#define ID_ALGO_HERMITE 3006
#define ID_ALGO_COHEN_SUTHERLAND 3007
#define ID_ALGO_LIANG_BARSKY 3008

// Fill methods
#define ID_FILL_LINES 4001
#define ID_FILL_CIRCLES 4002

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
HWND g_hColorButton;
HWND g_hThicknessSlider;
HWND g_hShapeLabel;
HWND g_hAlgorithmLabel;
HWND g_hThicknessLabel;
HWND g_hColorLabel;
std::wstring g_htmlPath;
HBRUSH g_hBackgroundBrush;
HBRUSH g_hCanvasBrush;
HFONT g_hLabelFont;
HFONT g_hButtonFont;

// Drawing state
COLORREF g_currentColor = COLOR_BLACK;
int g_currentTool = ID_TOOL_LINE;
int g_currentAlgorithm = ID_ALGO_DDA;
int g_currentFill = ID_FILL_LINES;
int g_lineThickness = 1;
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
void DrawCircle(HDC hdc, int centerX, int centerY, int radius, COLORREF color, int thickness);
void DrawEllipse(HDC hdc, int centerX, int centerY, int radiusX, int radiusY, COLORREF color, int thickness);
void ClearCanvas(HDC hdc);
void SaveCanvasToBitmap(HWND hwnd);
void ShowColorDialog(HWND hwnd);
void UpdateColorButton();
RECT GetCanvasRect(HWND hwnd);
void UpdateControlsFromSelection();
void HandleShapeSelection();
void HandleAlgorithmSelection();
HFONT CreateCustomFont(int size, bool bold);
void UpdateAlgorithmDropdown();
void UpdateInstructions();

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
    AppendMenuW(g_hMenuBar, MF_POPUP, (UINT_PTR)g_hAlgoMenu, L"&Algorithms");

    // Create Fill menu
    g_hFillMenu = CreateMenu();
    AppendMenuW(g_hFillMenu, MF_STRING | MF_CHECKED, ID_FILL_LINES, L"Fill with &Lines");
    AppendMenuW(g_hFillMenu, MF_STRING, ID_FILL_CIRCLES, L"Fill with &Circles");
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
    TBBUTTON tbb[6];
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

    // Add buttons to toolbar
    SendMessage(g_hToolbar, TB_ADDSTRING, 0, (LPARAM)L"Line\0Circle\0Ellipse\0Curve\0Polygon\0Clipping\0\0");
    SendMessage(g_hToolbar, TB_ADDBUTTONS, 6, (LPARAM)&tbb);

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
    
    // Set font for labels
    SendMessage(g_hShapeLabel, WM_SETFONT, (WPARAM)g_hLabelFont, TRUE);
    SendMessage(g_hAlgorithmLabel, WM_SETFONT, (WPARAM)g_hLabelFont, TRUE);
    SendMessage(g_hColorLabel, WM_SETFONT, (WPARAM)g_hLabelFont, TRUE);
    SendMessage(g_hThicknessLabel, WM_SETFONT, (WPARAM)g_hLabelFont, TRUE);
    
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
    
    // Set font for controls
    SendMessage(g_hShapeCombo, WM_SETFONT, (WPARAM)g_hLabelFont, TRUE);
    SendMessage(g_hAlgorithmCombo, WM_SETFONT, (WPARAM)g_hLabelFont, TRUE);
    SendMessage(g_hColorButton, WM_SETFONT, (WPARAM)g_hButtonFont, TRUE);
    
    // Set range and position for thickness slider
    SendMessage(g_hThicknessSlider, TBM_SETRANGE, TRUE, MAKELONG(1, 10));
    SendMessage(g_hThicknessSlider, TBM_SETPOS, TRUE, g_lineThickness);
    
    // Update color button appearance
    UpdateColorButton();
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
            
            // Reset ellipse click counter if changing from ellipse
            if (g_currentTool != ID_TOOL_ELLIPSE) {
                g_ellipseClickCount = 0;
            }
            
            // Update the algorithm dropdown to show appropriate algorithms for this tool
            UpdateAlgorithmDropdown();
            
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
            {
                // Uncheck previous algorithm
                CheckMenuItem(g_hAlgoMenu, g_currentAlgorithm, MF_BYCOMMAND | MF_UNCHECKED);
                // Update current algorithm
                g_currentAlgorithm = LOWORD(wParam);
                // Check new algorithm
                CheckMenuItem(g_hAlgoMenu, g_currentAlgorithm, MF_BYCOMMAND | MF_CHECKED);
                
                // Explicitly update the algorithm combo box selection
                int algoIndex = 0;
                switch (g_currentAlgorithm) {
                    case ID_ALGO_DDA: algoIndex = 0; break;
                    case ID_ALGO_MIDPOINT: algoIndex = 1; break;
                    case ID_ALGO_PARAMETRIC: algoIndex = 2; break;
                    case ID_ALGO_POLAR: algoIndex = 3; break;
                }
                SendMessage(g_hAlgorithmCombo, CB_SETCURSEL, algoIndex, 0);
                
                // Force redraw of the algorithm combo box
                InvalidateRect(g_hAlgorithmCombo, NULL, TRUE);
                UpdateWindow(g_hAlgorithmCombo);
            }
            break;

        // Fill selection
        case ID_FILL_LINES:
        case ID_FILL_CIRCLES:
            // Uncheck previous fill
            CheckMenuItem(g_hFillMenu, g_currentFill, MF_BYCOMMAND | MF_UNCHECKED);
            // Update current fill
            g_currentFill = LOWORD(wParam);
            // Check new fill
            CheckMenuItem(g_hFillMenu, g_currentFill, MF_BYCOMMAND | MF_CHECKED);
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
        case ID_ALGO_COHEN_SUTHERLAND: algoName = L"Cohen-Sutherland"; break;
        case ID_ALGO_LIANG_BARSKY: algoName = L"Liang-Barsky"; break;
        default: algoName = L"Unknown"; break;
    }
    
    _snwprintf(statusText, 256, L"Current tool: %s, Algorithm: %s, Thickness: %d", 
             toolName, algoName, g_lineThickness);
    UpdateStatusBar(statusText);
}

void DrawLine(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color, int thickness) {
    // For demonstration, we'll use the GDI line drawing
    HPEN hPen = CreatePen(PS_SOLID, thickness, color);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    
    MoveToEx(hdc, x1, y1, NULL);
    LineTo(hdc, x2, y2);
    
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

void DrawCircle(HDC hdc, int centerX, int centerY, int radius, COLORREF color, int thickness) {
    // For demonstration, we'll use the GDI ellipse drawing
    HPEN hPen = CreatePen(PS_SOLID, thickness, color);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    
    Ellipse(hdc, centerX - radius, centerY - radius, centerX + radius, centerY + radius);
    
    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hPen);
}

void DrawEllipse(HDC hdc, int centerX, int centerY, int radiusX, int radiusY, COLORREF color, int thickness) {
    // For demonstration, we'll use the GDI ellipse drawing
    HPEN hPen = CreatePen(PS_SOLID, thickness, color);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    
    Ellipse(hdc, centerX - radiusX, centerY - radiusY, centerX + radiusX, centerY + radiusY);
    
    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hPen);
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
    
    // Reset ellipse click counter if changing from ellipse
    if (g_currentTool != ID_TOOL_ELLIPSE) {
        g_ellipseClickCount = 0;
    }
    
    // Update the algorithm dropdown to show appropriate algorithms for this tool
    UpdateAlgorithmDropdown();
    
    // Update instructions in the status bar
    UpdateInstructions();
}

void HandleAlgorithmSelection() {
    // Get the current selection
    int selection = (int)SendMessage(g_hAlgorithmCombo, CB_GETCURSEL, 0, 0);
    
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
                case 1: algoID = ID_ALGO_PARAMETRIC; break;
                default: algoID = ID_ALGO_MIDPOINT; break;
            }
            break;
            
        case ID_TOOL_CURVE:
            switch (selection) {
                case 0: algoID = ID_ALGO_BEZIER; break;
                case 1: algoID = ID_ALGO_HERMITE; break;
                default: algoID = ID_ALGO_BEZIER; break;
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
    
    // No need to update the combo box itself as it's already set
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
    }
    SendMessage(g_hShapeCombo, CB_SETCURSEL, shapeIndex, 0);
    
    // Update the algorithm dropdown based on current algorithm
    int algoIndex = 0;
    switch (g_currentAlgorithm) {
        case ID_ALGO_DDA: algoIndex = 0; break;
        case ID_ALGO_MIDPOINT: algoIndex = 1; break;
        case ID_ALGO_PARAMETRIC: algoIndex = 2; break;
        case ID_ALGO_POLAR: algoIndex = 3; break;
    }
    SendMessage(g_hAlgorithmCombo, CB_SETCURSEL, algoIndex, 0);
    
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
            SendMessage(g_hAlgorithmCombo, CB_ADDSTRING, 0, (LPARAM)L"Parametric");
            break;
            
        case ID_TOOL_CURVE:
            SendMessage(g_hAlgorithmCombo, CB_ADDSTRING, 0, (LPARAM)L"Bezier");
            SendMessage(g_hAlgorithmCombo, CB_ADDSTRING, 0, (LPARAM)L"Hermite");
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
        (g_currentTool == ID_TOOL_POLYGON) ? L"Polygon" : L"Clipping";
    
    const wchar_t* algoName;
    switch (g_currentAlgorithm) {
        case ID_ALGO_DDA: algoName = L"DDA"; break;
        case ID_ALGO_MIDPOINT: algoName = L"Midpoint"; break;
        case ID_ALGO_PARAMETRIC: algoName = L"Parametric"; break;
        case ID_ALGO_POLAR: algoName = L"Polar"; break;
        case ID_ALGO_BEZIER: algoName = L"Bezier"; break;
        case ID_ALGO_HERMITE: algoName = L"Hermite"; break;
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
            instructions = L"Click for control points, double-click to finish curve";
            break;
        case ID_TOOL_POLYGON:
            instructions = L"Click for each vertex, right-click to close polygon";
            break;
        case ID_TOOL_CLIP:
            instructions = L"Click and drag to define clipping window";
            break;
        default:
            instructions = L"Select a tool to begin drawing";
            break;
    }
    
    _snwprintf(statusText, 256, L"%s: %s. Tool: %s, Algorithm: %s, Thickness: %d", 
             toolName, instructions, toolName, algoName, g_lineThickness);
    UpdateStatusBar(statusText);
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
                        return 0;
                    }
                    else if (LOWORD(wParam) == ID_COMBO_ALGORITHM) {
                        HandleAlgorithmSelection();
                        return 0;
                    }
                    break;
                    
                case BN_CLICKED:
                    // Handle button clicks
                    if (LOWORD(wParam) == ID_COLOR_BUTTON) {
                        ShowColorDialog(hwnd);
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

                // Recreate the drawing buffer
                CleanupDrawingBuffer();
                InitializeDrawingBuffer(hwnd);
            }
            return 0;

        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                
                // Draw the canvas border with a nice 3D effect
                HPEN hPen = CreatePen(PS_SOLID, 1, COLOR_DARKGRAY);
                HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
                
                // Draw a nice 3D frame around the canvas
                DrawEdge(hdc, &g_canvasRect, EDGE_SUNKEN, BF_RECT);
                
                // Copy the memory DC to the window DC
                BitBlt(hdc, g_canvasRect.left, g_canvasRect.top, 
                       g_canvasRect.right - g_canvasRect.left, 
                       g_canvasRect.bottom - g_canvasRect.top, 
                       g_hdcMem, 0, 0, SRCCOPY);
                
                // Draw temporary guidelines for shapes in progress
                if (g_isDrawing) {
                    // Convert to screen coordinates
                    int startX = g_startPoint.x + g_canvasRect.left;
                    int startY = g_startPoint.y + g_canvasRect.top;
                    int endX = g_endPoint.x + g_canvasRect.left;
                    int endY = g_endPoint.y + g_canvasRect.top;
                    
                    // Create dashed pen for guidelines
                    LOGBRUSH lb;
                    lb.lbStyle = BS_SOLID;
                    lb.lbColor = g_currentColor;
                    lb.lbHatch = 0;
                    HPEN hDashedPen = ExtCreatePen(PS_COSMETIC | PS_DASH, 1, &lb, 0, NULL);
                    HPEN hOldGuidePen = (HPEN)SelectObject(hdc, hDashedPen);
                    
                    switch (g_currentTool) {
                        case ID_TOOL_LINE:
                            MoveToEx(hdc, startX, startY, NULL);
                            LineTo(hdc, endX, endY);
                            break;
                            
                        case ID_TOOL_CIRCLE:
                            {
                                int dx = endX - startX;
                                int dy = endY - startY;
                                int radius = (int)sqrt((double)(dx*dx + dy*dy));
                                
                                // Draw temporary circle
                                HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
                                Ellipse(hdc, startX - radius, startY - radius, startX + radius, startY + radius);
                                
                                // Draw a line showing the radius
                                MoveToEx(hdc, startX, startY, NULL);
                                LineTo(hdc, endX, endY);
                                
                                SelectObject(hdc, hOldBrush);
                            }
                            break;
                            
                        case ID_TOOL_CLIP:
                            {
                                // Draw temporary clipping rectangle
                                HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
                                Rectangle(hdc, startX, startY, endX, endY);
                                SelectObject(hdc, hOldBrush);
                            }
                            break;
                    }
                    
                    SelectObject(hdc, hOldGuidePen);
                    DeleteObject(hDashedPen);
                }
                
                // Draw temporary visualization for ellipse drawing
                if (g_currentTool == ID_TOOL_ELLIPSE && g_ellipseClickCount > 0) {
                    // Create dashed pen for guidelines
                    LOGBRUSH lb;
                    lb.lbStyle = BS_SOLID;
                    lb.lbColor = g_currentColor;
                    lb.lbHatch = 0;
                    HPEN hDashedPen = ExtCreatePen(PS_COSMETIC | PS_DASH, 1, &lb, 0, NULL);
                    HPEN hOldGuidePen = (HPEN)SelectObject(hdc, hDashedPen);
                    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
                    
                    // Convert to screen coordinates
                    int centerX = g_ellipseCenter.x + g_canvasRect.left;
                    int centerY = g_ellipseCenter.y + g_canvasRect.top;
                    
                    // Draw center point
                    Ellipse(hdc, centerX - 3, centerY - 3, centerX + 3, centerY + 3);
                    
                    if (g_ellipseClickCount >= 1) {
                        // First axis point
                        int point1X = g_ellipsePoint1.x + g_canvasRect.left;
                        int point1Y = g_ellipsePoint1.y + g_canvasRect.top;
                        
                        // Draw first point
                        Ellipse(hdc, point1X - 3, point1Y - 3, point1X + 3, point1Y + 3);
                        
                        // Draw guide line from center to first point
                        MoveToEx(hdc, centerX, centerY, NULL);
                        LineTo(hdc, point1X, point1Y);
                        
                        if (g_ellipseClickCount >= 2) {
                            // Second axis point
                            int point2X = g_ellipsePoint2.x + g_canvasRect.left;
                            int point2Y = g_ellipsePoint2.y + g_canvasRect.top;
                            
                            // Draw second point
                            Ellipse(hdc, point2X - 3, point2Y - 3, point2X + 3, point2Y + 3);
                            
                            // Draw guide line from center to second point
                            MoveToEx(hdc, centerX, centerY, NULL);
                            LineTo(hdc, point2X, point2Y);
                        }
                    }
                    
                    SelectObject(hdc, hOldGuidePen);
                    SelectObject(hdc, hOldBrush);
                    DeleteObject(hDashedPen);
                }
                
                // Clean up
                SelectObject(hdc, hOldPen);
                DeleteObject(hPen);
                
                EndPaint(hwnd, &ps);
            }
            return 0;

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
                        // Add point to polygon
                        POINT pt = {x, y};
                        g_polygonPoints.push_back(pt);
                        
                        // Draw a small point to mark the vertex
                        DrawCircle(g_hdcMem, x, y, 2, g_currentColor, 1);
                        
                        // If there are at least 2 points, draw a line between the last two
                        if (g_polygonPoints.size() >= 2) {
                            POINT p1 = g_polygonPoints[g_polygonPoints.size() - 2];
                            POINT p2 = g_polygonPoints[g_polygonPoints.size() - 1];
                            DrawLine(g_hdcMem, p1.x, p1.y, p2.x, p2.y, g_currentColor, g_lineThickness);
                        }
                        
                        // Invalidate the canvas area to redraw
                        InvalidateRect(hwnd, &g_canvasRect, FALSE);
                    }
                    else if (g_currentTool == ID_TOOL_ELLIPSE) {
                        // Handle 3-point ellipse drawing
                        if (g_ellipseClickCount == 0) {
                            // First click - store the center point
                            g_ellipseCenter.x = x;
                            g_ellipseCenter.y = y;
                            g_ellipseClickCount++;
                            
                            // Draw a small point to mark the center
                            DrawCircle(g_hdcMem, x, y, 2, g_currentColor, 1);
                            InvalidateRect(hwnd, &g_canvasRect, FALSE);
                            
                            // Update status bar with next instruction
                            UpdateEllipseStatus();
                        }
                        else if (g_ellipseClickCount == 1) {
                            // Second click - store first axis point
                            g_ellipsePoint1.x = x;
                            g_ellipsePoint1.y = y;
                            g_ellipseClickCount++;
                            
                            // Draw a small point
                            DrawCircle(g_hdcMem, x, y, 2, g_currentColor, 1);
                            InvalidateRect(hwnd, &g_canvasRect, FALSE);
                            
                            // Update status bar with next instruction
                            UpdateEllipseStatus();
                        }
                        else if (g_ellipseClickCount == 2) {
                            // Third click - store second axis point
                            g_ellipsePoint2.x = x;
                            g_ellipsePoint2.y = y;
                            
                            // Calculate a and b for the ellipse
                            g_ellipseA = std::max(abs(g_ellipsePoint1.x - g_ellipseCenter.x), abs(g_ellipsePoint2.x - g_ellipseCenter.x));
                            g_ellipseB = std::max(abs(g_ellipsePoint1.y - g_ellipseCenter.y), abs(g_ellipsePoint2.y - g_ellipseCenter.y));
                            
                            // Draw the ellipse using the selected algorithm
                            switch (g_currentAlgorithm) {
                                case ID_ALGO_MIDPOINT:
                                    // Draw ellipse using midpoint algorithm
                                    DrawEllipse(g_hdcMem, g_ellipseCenter.x, g_ellipseCenter.y, g_ellipseA, g_ellipseB, g_currentColor, g_lineThickness);
                                    break;
                                case ID_ALGO_PARAMETRIC:
                                    // Draw ellipse using parametric algorithm
                                    DrawEllipse(g_hdcMem, g_ellipseCenter.x, g_ellipseCenter.y, g_ellipseA, g_ellipseB, g_currentColor, g_lineThickness);
                                    break;
                                default:
                                    // Default ellipse drawing
                                    DrawEllipse(g_hdcMem, g_ellipseCenter.x, g_ellipseCenter.y, g_ellipseA, g_ellipseB, g_currentColor, g_lineThickness);
                                    break;
                            }
                            
                            // Reset click counter for next ellipse
                            g_ellipseClickCount = 0;
                            
                            // Invalidate the canvas area to redraw
                            InvalidateRect(hwnd, &g_canvasRect, FALSE);
                            
                            // Restore normal instructions
                            UpdateInstructions();
                        }
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
                if (g_isDrawing) {
                    int x = GET_X_LPARAM(lParam);
                    int y = GET_Y_LPARAM(lParam);
                    
                    // Convert to canvas coordinates
                    g_endPoint.x = x - g_canvasRect.left;
                    g_endPoint.y = y - g_canvasRect.top;
                    
                    // Redraw to show the current shape
                    InvalidateRect(hwnd, &g_canvasRect, FALSE);
                }
            }
            return 0;

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
                            DrawLine(g_hdcMem, g_startPoint.x, g_startPoint.y, g_endPoint.x, g_endPoint.y, g_currentColor, g_lineThickness);
                            break;
                            
                        case ID_TOOL_CIRCLE:
                            {
                                // Calculate radius
                                int dx = g_endPoint.x - g_startPoint.x;
                                int dy = g_endPoint.y - g_startPoint.y;
                                int radius = (int)sqrt((double)(dx*dx + dy*dy));
                                
                                DrawCircle(g_hdcMem, g_startPoint.x, g_startPoint.y, radius, g_currentColor, g_lineThickness);
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