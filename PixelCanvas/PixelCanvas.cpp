#include "framework.h"
#include "PixelCanvas.h"
#include "Line.h"
#include "Circle.h"
#include "Curve.h"
#include "Resource.h"
#include <commdlg.h>
#include "Ellipse.h"
#include "PolygonFill.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Canvas parameters
const int canvasWidth = 1200;
const int canvasHeight = 600;

// Pixel drawing globals
HBITMAP hBitmap = NULL;
BYTE* pPixels = NULL; // pointer to pixel bits
HDC hMemDC = NULL;    // memory DC for the bitmap
BITMAPINFO bmi = { 0 };

// Drawing state
enum DrawMode { MODE_NONE, MODE_LINE };
DrawMode currentMode = MODE_NONE;

// Line drawing points
POINT lineStart = { -1, -1 };
POINT lineEnd = { -1, -1 };

// Add a variable to track the current line algorithm
enum LineAlgorithm { LINE_DDA, LINE_MIDPOINT, LINE_PARAMETRIC };
LineAlgorithm currentLineAlgorithm = LINE_DDA;

// Add a global HWND for the combo box
HWND hComboAlgo = NULL;

// Add a constant for the top offset (height of the combo box)
const int topOffset = 40; // Adjust as needed for your combo box height

// Add a flag to track if we are waiting for the second right click
bool waitingForSecondClick = false;

// Add a global variable for the selected color
COLORREF g_LineColor = RGB(0, 0, 0);

// Add global HWNDs for the shape and algorithm combo boxes
HWND hComboShape = NULL;

// Add enums for shape and algorithm selection
enum ShapeType { SHAPE_LINE, SHAPE_CIRCLE, SHAPE_CIRCLE_QUARTER, SHAPE_SQUARE, SHAPE_RECTANGLE, SHAPE_FLOODFILL, SHAPE_CARDINAL_SPLINE, SHAPE_POLYGON, SHAPE_CLIP_WINDOW, SHAPE_ELLIPSE };
ShapeType currentShape = SHAPE_LINE;

// Add this near the top of the file, after global variables
static HBRUSH hDarkMenuBrush = NULL;

// Add global variables for storing up to 4 points
POINT shapePoints[4];
int shapeClickCount = 0;

// Add a global variable for the fill color
COLORREF g_FillColor = RGB(255, 0, 0);

// Add global variables for Cardinal Spline
POINT splinePoints[100];
int splinePointCount = 0;
HWND hBtnFinishSpline = NULL;

// Add global variables for Polygon
POINT polygonPoints[100];
int polygonPointCount = 0;
HWND hBtnFinishPolygon = NULL;

// Clipping window state
POINT clipWindowPoints[4];
bool clipWindowSet = false;
bool clipWindowIsSquare = false;
int clipMinX, clipMaxX, clipMinY, clipMaxY;

void UpdateClipWindowBounds() {
    clipMinX = min(min(clipWindowPoints[0].x, clipWindowPoints[1].x), min(clipWindowPoints[2].x, clipWindowPoints[3].x));
    clipMaxX = max(max(clipWindowPoints[0].x, clipWindowPoints[1].x), max(clipWindowPoints[2].x, clipWindowPoints[3].x));
    clipMinY = min(min(clipWindowPoints[0].y, clipWindowPoints[1].y), min(clipWindowPoints[2].y, clipWindowPoints[3].y));
    clipMaxY = max(max(clipWindowPoints[0].y, clipWindowPoints[1].y), max(clipWindowPoints[2].y, clipWindowPoints[3].y));
    if (clipWindowIsSquare) {
        int side = max(clipMaxX - clipMinX, clipMaxY - clipMinY);
        clipMaxX = clipMinX + side;
        clipMaxY = clipMinY + side;
    }
}

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void DrawPixel(int x, int y, COLORREF color);
void DrawLineOnBitmap(POINT start, POINT end, COLORREF color);
void DrawPreviewLine(HWND hWnd, HDC hdc, POINT start, POINT end);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PIXELCANVAS, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PIXELCANVAS));

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PIXELCANVAS));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_PIXELCANVAS);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    // Create window with fixed size + borders for canvas
    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, canvasWidth + 16, canvasHeight + 39, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

void DrawPixel(int x, int y, COLORREF color)
{
    if (x < 0 || x >= canvasWidth || y < 0 || y >= canvasHeight || pPixels == nullptr)
        return;

    BYTE* pixel = pPixels + (y * canvasWidth + x) * 4;
    pixel[0] = GetBValue(color);
    pixel[1] = GetGValue(color);
    pixel[2] = GetRValue(color);
    pixel[3] = 255;
}

// Bresenham's line algorithm for drawing on bitmap pixels
void DrawLineOnBitmap(POINT start, POINT end, COLORREF color)
{
    int x0 = start.x;
    int y0 = start.y;
    int x1 = end.x;
    int y1 = end.y;

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true)
    {
        DrawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1)
            break;
        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

void DrawPreviewLine(HWND hWnd, HDC hdc, POINT start, POINT end)
{
    // Draw rubber-band line in XOR mode to show preview
    SetROP2(hdc, R2_NOTXORPEN);
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

    MoveToEx(hdc, start.x, start.y, NULL);
    LineTo(hdc, end.x, end.y);

    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        HDC hdc = GetDC(hWnd);
        hMemDC = CreateCompatibleDC(hdc);
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = canvasWidth;
        bmi.bmiHeader.biHeight = -canvasHeight; // top-down
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        hBitmap = CreateDIBSection(hMemDC, &bmi, DIB_RGB_COLORS, (void**)&pPixels, NULL, 0);
        SelectObject(hMemDC, hBitmap);
        if (pPixels)
            memset(pPixels, 255, canvasWidth * canvasHeight * 4); // Set background to white
        ReleaseDC(hWnd, hdc);

        // Create the shape selection combo box
        hComboShape = CreateWindowW(L"COMBOBOX", NULL, CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP,
            10, 10, 120, 200, hWnd, (HMENU)IDC_COMBO_SHAPE, hInst, NULL);
        SendMessageW(hComboShape, CB_ADDSTRING, 0, (LPARAM)L"Line");
        SendMessageW(hComboShape, CB_ADDSTRING, 0, (LPARAM)L"Circle");
        SendMessageW(hComboShape, CB_ADDSTRING, 0, (LPARAM)L"Circle with Quarter Filled");
        SendMessageW(hComboShape, CB_ADDSTRING, 0, (LPARAM)L"Square");
        SendMessageW(hComboShape, CB_ADDSTRING, 0, (LPARAM)L"Rectangle");
        SendMessageW(hComboShape, CB_ADDSTRING, 0, (LPARAM)L"Flood Fill");
        SendMessageW(hComboShape, CB_ADDSTRING, 0, (LPARAM)L"Cardinal Spline");
        SendMessageW(hComboShape, CB_ADDSTRING, 0, (LPARAM)L"Polygon");
        SendMessageW(hComboShape, CB_ADDSTRING, 0, (LPARAM)L"Clipping Window");
        SendMessageW(hComboShape, CB_ADDSTRING, 0, (LPARAM)L"Ellipse");
        SendMessageW(hComboShape, CB_SETCURSEL, 0, 0); // Default to Line

        // Create the algorithm selection combo box (move it to the right of shape combo)
        hComboAlgo = CreateWindowW(L"COMBOBOX", NULL, CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP,
            140, 10, 150, 200, hWnd, (HMENU)IDC_COMBO_ALGO, hInst, NULL);
        // Add line algorithms by default
        SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"DDA");
        SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Midpoint");
        SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Parametric");
        SendMessageW(hComboAlgo, CB_SETCURSEL, 0, 0);

        // Combo boxes: shape (10,10,120,200), algo (140,10,150,200)
        int buttonStartX = 310; // right of both combo boxes
        int buttonY = 10;
        int buttonH = 24;
        int buttonW1 = 80; // Save, Load
        int buttonW2 = 120; // Clear, Color, Fill Color
        int gap = 10;
        // Add the 'Save' button first
        HWND hBtnSave = CreateWindowW(L"BUTTON", L"Save", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            buttonStartX, buttonY, buttonW1, buttonH, hWnd, (HMENU)4004, hInst, NULL);
        // Add the 'Load' button next
        HWND hBtnLoad = CreateWindowW(L"BUTTON", L"Load", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            buttonStartX + buttonW1 + gap, buttonY, buttonW1, buttonH, hWnd, (HMENU)4005, hInst, NULL);
        // Add the 'Clear Screen' button
        HWND hBtnClear = CreateWindowW(L"BUTTON", L"Clear Screen", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            buttonStartX + 2 * (buttonW1 + gap), buttonY, buttonW2, buttonH, hWnd, (HMENU)4003, hInst, NULL);
        // Add the 'Choose Color' button
        HWND hBtnColor = CreateWindowW(L"BUTTON", L"Choose Color", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            buttonStartX + 2 * (buttonW1 + gap) + buttonW2 + gap, buttonY, buttonW2, buttonH, hWnd, (HMENU)IDC_BTN_COLOR, hInst, NULL);
        // Add the 'Fill Color' button (if it appears)
        HWND hBtnFillColor = CreateWindowW(L"BUTTON", L"Choose Fill Color", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            buttonStartX + 2 * (buttonW1 + gap) + 2 * (buttonW2 + gap), buttonY, buttonW2, buttonH, hWnd, (HMENU)IDC_BTN_FILL_COLOR, hInst, NULL);
        ShowWindow(hBtnFillColor, SW_HIDE); // Hide by default
        // Add the 'Finish Polygon' button to the right of fill color
        hBtnFinishPolygon = CreateWindowW(L"BUTTON", L"Finish Polygon", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            570, 10, 120, 24, hWnd, (HMENU)4002, hInst, NULL);
        ShowWindow(hBtnFinishPolygon, SW_HIDE);

        // Add the 'Finish Spline' button (hidden by default) right next to the 'Choose Fill Color' button, with the same gap
        hBtnFinishSpline = CreateWindowW(L"BUTTON", L"Finish Spline", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            buttonStartX + 2 * (buttonW1 + gap) + 3 * (buttonW2 + gap), buttonY, buttonW2, buttonH, hWnd, (HMENU)4001, hInst, NULL);
        ShowWindow(hBtnFinishSpline, SW_HIDE);
    }
    break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Handle shape combo box selection change
        if (wmId == IDC_COMBO_SHAPE && HIWORD(wParam) == CBN_SELCHANGE) {
            int sel = (int)SendMessageW(hComboShape, CB_GETCURSEL, 0, 0);
            if (sel == 0) currentShape = SHAPE_LINE;
            else if (sel == 1) currentShape = SHAPE_CIRCLE;
            else if (sel == 2) currentShape = SHAPE_CIRCLE_QUARTER;
            else if (sel == 3) currentShape = SHAPE_SQUARE;
            else if (sel == 4) currentShape = SHAPE_RECTANGLE;
            else if (sel == 5) currentShape = SHAPE_FLOODFILL;
            else if (sel == 6) currentShape = SHAPE_CARDINAL_SPLINE;
            else if (sel == 7) currentShape = SHAPE_POLYGON;
            else if (sel == 8) currentShape = SHAPE_CLIP_WINDOW;
            else if (sel == 9) currentShape = SHAPE_ELLIPSE;
            // Update algorithm combo box
            SendMessageW(hComboAlgo, CB_RESETCONTENT, 0, 0);
            if (currentShape == SHAPE_LINE) {
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"DDA");
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Midpoint");
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Parametric");
            } else if (currentShape == SHAPE_CIRCLE) {
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Direct");
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Polar");
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Iterative Polar");
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Midpoint");
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Modified Midpoint");
            } else if (currentShape == SHAPE_CIRCLE_QUARTER) {
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Fill Quarter 1 With Circles");
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Fill Quarter 2 With Circles");
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Fill Quarter 3 With Circles");
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Fill Quarter 4 With Circles");
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Fill Quarter 1 With Lines");
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Fill Quarter 2 With Lines");
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Fill Quarter 3 With Lines");
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Fill Quarter 4 With Lines");
            } else if (currentShape == SHAPE_SQUARE) {
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Fill with Hermite");
            } else if (currentShape == SHAPE_RECTANGLE) {
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Fill with Bezier");
            } else if (currentShape == SHAPE_FLOODFILL) {
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Recursive");
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Non-Recursive");
            } else if (currentShape == SHAPE_CARDINAL_SPLINE) {
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Cardinal Spline");
                ShowWindow(hBtnFinishSpline, SW_SHOW);
                splinePointCount = 0;
                ShowWindow(hBtnFinishPolygon, SW_HIDE);
            } else if (currentShape == SHAPE_POLYGON) {
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Convex Fill");
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"General Fill");
                ShowWindow(hBtnFinishPolygon, SW_SHOW);
                polygonPointCount = 0;
                ShowWindow(hBtnFinishSpline, SW_HIDE);
            } else if (currentShape == SHAPE_CLIP_WINDOW) {
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Rectangle");
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Square");
                SendMessageW(hComboAlgo, CB_SETCURSEL, 0, 0);
            } else if (currentShape == SHAPE_ELLIPSE) {
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Direct");
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Polar");
                SendMessageW(hComboAlgo, CB_ADDSTRING, 0, (LPARAM)L"Midpoint");
                SendMessageW(hComboAlgo, CB_SETCURSEL, 0, 0);
            }
            // Show/hide fill color button based on shape
            bool showFill = false;
            if (currentShape == SHAPE_POLYGON || currentShape == SHAPE_FLOODFILL || currentShape == SHAPE_SQUARE || currentShape == SHAPE_RECTANGLE || currentShape == SHAPE_CIRCLE_QUARTER || currentShape == SHAPE_ELLIPSE) {
                showFill = true;
            } else if (currentShape == SHAPE_CIRCLE) {
                int algoCount = (int)SendMessageW(hComboAlgo, CB_GETCOUNT, 0, 0);
                int selAlgo = (int)SendMessageW(hComboAlgo, CB_GETCURSEL, 0, 0);
                if (selAlgo >= 5) showFill = true;
            }
            ShowWindow(GetDlgItem(hWnd, IDC_BTN_FILL_COLOR), showFill ? SW_SHOW : SW_HIDE);
            // Show/hide finish buttons
            ShowWindow(hBtnFinishPolygon, (currentShape == SHAPE_POLYGON) ? SW_SHOW : SW_HIDE);
            ShowWindow(hBtnFinishSpline, (currentShape == SHAPE_CARDINAL_SPLINE) ? SW_SHOW : SW_HIDE);
            shapeClickCount = 0;
            return 0;
        }
        if (wmId == IDC_BTN_COLOR) {
            CHOOSECOLOR cc = { sizeof(CHOOSECOLOR) };
            static COLORREF customColors[16] = { 0 };
            cc.hwndOwner = hWnd;
            cc.lpCustColors = customColors;
            cc.rgbResult = g_LineColor;
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;
            if (ChooseColor(&cc)) {
                g_LineColor = cc.rgbResult;
            }
            return 0;
        }
        if (wmId == IDC_BTN_FILL_COLOR) {
            CHOOSECOLOR cc = { sizeof(CHOOSECOLOR) };
            static COLORREF customColors[16] = { 0 };
            cc.hwndOwner = hWnd;
            cc.lpCustColors = customColors;
            cc.rgbResult = g_FillColor;
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;
            if (ChooseColor(&cc)) {
                g_FillColor = cc.rgbResult;
            }
            return 0;
        }
        // Handle Finish Spline button
        if (wmId == 4001) {
            if (splinePointCount >= 4) {
                Curve curve(hMemDC);
                double s = 0.0; // Tension parameter (0 = Catmull-Rom)
                for (int i = 1; i < splinePointCount - 2; ++i) {
                    int x0 = splinePoints[i].x;
                    int y0 = splinePoints[i].y;
                    int x1 = splinePoints[i + 1].x;
                    int y1 = splinePoints[i + 1].y;
                    int t0 = (int)(((1 - s) / 2) * (splinePoints[i + 1].x - splinePoints[i - 1].x));
                    int t1 = (int)(((1 - s) / 2) * (splinePoints[i + 2].x - splinePoints[i].x));
                    curve.DrawHermite(x0, y0, x1, y1, t0, t1, g_LineColor);
                }
                InvalidateRect(hWnd, NULL, FALSE);
            }
            splinePointCount = 0;
            ShowWindow(hBtnFinishSpline, SW_HIDE);
            return 0;
        }
        // Handle Finish Polygon button
        if (wmId == 4002) {
            if (polygonPointCount >= 3) {
                int algoSel = (int)SendMessageW(hComboAlgo, CB_GETCURSEL, 0, 0);
                point pts[100];
                for (int i = 0; i < polygonPointCount; ++i) {
                    pts[i] = point(polygonPoints[i].x, polygonPoints[i].y);
                }
                if (algoSel == 0) {
                    convexfill(hMemDC, pts, polygonPointCount, g_FillColor);
                } else {
                    fillGeneralPolygon(hMemDC, pts, polygonPointCount, g_FillColor);
                }
                InvalidateRect(hWnd, NULL, FALSE);
            }
            polygonPointCount = 0;
            ShowWindow(hBtnFinishPolygon, SW_HIDE);
            return 0;
        }
        // Handle algorithm combo box selection change
        if (wmId == IDC_COMBO_ALGO && HIWORD(wParam) == CBN_SELCHANGE) {
            // Show/hide fill color button based on shape and algorithm
            bool showFill = false;
            if (currentShape == SHAPE_POLYGON || currentShape == SHAPE_FLOODFILL || currentShape == SHAPE_SQUARE || currentShape == SHAPE_RECTANGLE || currentShape == SHAPE_CIRCLE_QUARTER || currentShape == SHAPE_ELLIPSE) {
                showFill = true;
            } else if (currentShape == SHAPE_CIRCLE) {
                int selAlgo = (int)SendMessageW(hComboAlgo, CB_GETCURSEL, 0, 0);
                if (selAlgo >= 5) showFill = true;
            }
            ShowWindow(GetDlgItem(hWnd, IDC_BTN_FILL_COLOR), showFill ? SW_SHOW : SW_HIDE);
        }
        if (wmId == 4003) { // Clear Screen button
            if (pPixels) {
                memset(pPixels, 255, canvasWidth * canvasHeight * 4); // Set background to white
                clipWindowSet = false; // Remove any clipping restrictions
                InvalidateRect(hWnd, NULL, FALSE);
            }
            return 0;
        }
        if (wmId == 4004) { // Save button
            OPENFILENAME ofn = { sizeof(OPENFILENAME) };
            WCHAR szFile[MAX_PATH] = L"";
            ofn.hwndOwner = hWnd;
            ofn.lpstrFilter = L"Bitmap Files (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0";
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
            ofn.lpstrDefExt = L"bmp";
            if (GetSaveFileName(&ofn)) {
                // Save the bitmap to file
                BITMAPFILEHEADER bfh = { 0 };
                BITMAPINFOHEADER bih = bmi.bmiHeader;
                DWORD dwBmpSize = canvasWidth * canvasHeight * 4;
                bfh.bfType = 0x4D42; // 'BM'
                bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
                bfh.bfSize = bfh.bfOffBits + dwBmpSize;
                HANDLE hFile = CreateFile(ofn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE) {
                    DWORD dwWritten;
                    WriteFile(hFile, &bfh, sizeof(bfh), &dwWritten, NULL);
                    WriteFile(hFile, &bih, sizeof(bih), &dwWritten, NULL);
                    WriteFile(hFile, pPixels, dwBmpSize, &dwWritten, NULL);
                    CloseHandle(hFile);
                }
                // Save clipping state to .txt
                WCHAR txtFile[MAX_PATH];
                wcscpy_s(txtFile, szFile);
                WCHAR* dot = wcsrchr(txtFile, L'.');
                if (dot) wcscpy_s(dot, MAX_PATH - (dot - txtFile), L".txt");
                else wcscat_s(txtFile, MAX_PATH, L".txt");
                FILE* f = nullptr;
                if (_wfopen_s(&f, txtFile, L"w") == 0 && f) {
                    fprintf(f, "%d %d ", clipWindowSet ? 1 : 0, clipWindowIsSquare ? 1 : 0);
                    for (int i = 0; i < 4; ++i) fprintf(f, "%d %d ", clipWindowPoints[i].x, clipWindowPoints[i].y);
                    fprintf(f, "\n");
                    fclose(f);
                }
            }
            return 0;
        }
        if (wmId == 4005) { // Load button
            OPENFILENAME ofn = { sizeof(OPENFILENAME) };
            WCHAR szFile[MAX_PATH] = L"";
            ofn.hwndOwner = hWnd;
            ofn.lpstrFilter = L"Bitmap Files (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0";
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
            ofn.lpstrDefExt = L"bmp";
            if (GetOpenFileName(&ofn)) {
                HANDLE hFile = CreateFile(ofn.lpstrFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE) {
                    BITMAPFILEHEADER bfh;
                    BITMAPINFOHEADER bih;
                    DWORD dwRead;
                    ReadFile(hFile, &bfh, sizeof(bfh), &dwRead, NULL);
                    ReadFile(hFile, &bih, sizeof(bih), &dwRead, NULL);
                    if (bfh.bfType == 0x4D42 && bih.biWidth == canvasWidth && abs(bih.biHeight) == canvasHeight && bih.biBitCount == 32) {
                        ReadFile(hFile, pPixels, canvasWidth * canvasHeight * 4, &dwRead, NULL);
                        InvalidateRect(hWnd, NULL, FALSE);
                    }
                    CloseHandle(hFile);
                }
                // Load clipping state from .txt
                WCHAR txtFile[MAX_PATH];
                wcscpy_s(txtFile, szFile);
                WCHAR* dot = wcsrchr(txtFile, L'.');
                if (dot) wcscpy_s(dot, MAX_PATH - (dot - txtFile), L".txt");
                else wcscat_s(txtFile, MAX_PATH, L".txt");
                FILE* f = nullptr;
                if (_wfopen_s(&f, txtFile, L"r") == 0 && f) {
                    int set, square;
                    int pts[8];
                    if (fscanf_s(f, "%d %d %d %d %d %d %d %d %d %d %d %d", &set, &square, &pts[0], &pts[1], &pts[2], &pts[3], &pts[4], &pts[5], &pts[6], &pts[7]) >= 10) {
                        clipWindowSet = (set != 0);
                        clipWindowIsSquare = (square != 0);
                        for (int i = 0; i < 4; ++i) {
                            clipWindowPoints[i].x = pts[i * 2];
                            clipWindowPoints[i].y = pts[i * 2 + 1];
                        }
                        UpdateClipWindowBounds();
                    }
                    fclose(f);
                }
            }
            return 0;
        }
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;

        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    case WM_LBUTTONDOWN:
    {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam) - topOffset;
        if (y < 0 || x < 0 || x >= canvasWidth || y >= canvasHeight)
            break;
        if (currentShape == SHAPE_SQUARE || currentShape == SHAPE_RECTANGLE) {
            shapePoints[shapeClickCount].x = x;
            shapePoints[shapeClickCount].y = y;
            shapeClickCount++;
            if (shapeClickCount < 4) return 0;
            // On the 4th click, fill the shape
            int algoSel = (int)SendMessageW(hComboAlgo, CB_GETCURSEL, 0, 0);
            Curve curve(hMemDC);
            if (currentShape == SHAPE_SQUARE) {
                // Find bounding box and make it a square
                int minX = min(min(shapePoints[0].x, shapePoints[1].x), min(shapePoints[2].x, shapePoints[3].x));
                int maxX = max(max(shapePoints[0].x, shapePoints[1].x), max(shapePoints[2].x, shapePoints[3].x));
                int minY = min(min(shapePoints[0].y, shapePoints[1].y), min(shapePoints[2].y, shapePoints[3].y));
                int maxY = max(max(shapePoints[0].y, shapePoints[1].y), max(shapePoints[2].y, shapePoints[3].y));
                int side = max(maxX - minX, maxY - minY);
                curve.FillWithHermite(minX, minY, minX + side, minY + side, g_LineColor);
                // Draw borders
                Line line(hMemDC);
                line.DrawLineDDA(minX, minY, minX + side, minY, g_LineColor); // Top
                line.DrawLineDDA(minX + side, minY, minX + side, minY + side, g_LineColor); // Right
                line.DrawLineDDA(minX + side, minY + side, minX, minY + side, g_LineColor); // Bottom
                line.DrawLineDDA(minX, minY + side, minX, minY, g_LineColor); // Left
            } else if (currentShape == SHAPE_RECTANGLE) {
                // Find bounding box
                int minX = min(min(shapePoints[0].x, shapePoints[1].x), min(shapePoints[2].x, shapePoints[3].x));
                int maxX = max(max(shapePoints[0].x, shapePoints[1].x), max(shapePoints[2].x, shapePoints[3].x));
                int minY = min(min(shapePoints[0].y, shapePoints[1].y), min(shapePoints[2].y, shapePoints[3].y));
                int maxY = max(max(shapePoints[0].y, shapePoints[1].y), max(shapePoints[2].y, shapePoints[3].y));
                curve.FillWithBezier(minX, minY, maxX, maxY, g_LineColor);
                // Draw borders
                Line line(hMemDC);
                line.DrawLineDDA(minX, minY, maxX, minY, g_LineColor); // Top
                line.DrawLineDDA(maxX, minY, maxX, maxY, g_LineColor); // Right
                line.DrawLineDDA(maxX, maxY, minX, maxY, g_LineColor); // Bottom
                line.DrawLineDDA(minX, maxY, minX, minY, g_LineColor); // Left
            }
            InvalidateRect(hWnd, NULL, FALSE);
            shapeClickCount = 0;
            return 0;
        }
        if (currentShape == SHAPE_FLOODFILL) {
            int algoSel = (int)SendMessageW(hComboAlgo, CB_GETCURSEL, 0, 0);
            COLORREF boundaryColor = g_LineColor; // Use the current drawing color as the boundary
            if (algoSel == 0) {
                myFloodFill(hMemDC, x, y, boundaryColor, g_FillColor);
            } else {
                myFloodFillqueue(hMemDC, x, y, boundaryColor, g_FillColor);
            }
            InvalidateRect(hWnd, NULL, FALSE);
            return 0;
        }
        if (currentShape == SHAPE_CARDINAL_SPLINE) {
            if (splinePointCount < 100) {
                splinePoints[splinePointCount].x = x;
                splinePoints[splinePointCount].y = y;
                splinePointCount++;
            }
            return 0;
        }
        if (currentShape == SHAPE_POLYGON) {
            if (polygonPointCount < 100) {
                polygonPoints[polygonPointCount].x = x;
                polygonPoints[polygonPointCount].y = y;
                polygonPointCount++;
            }
            return 0;
        }
        if (currentShape == SHAPE_CLIP_WINDOW) {
            clipWindowPoints[shapeClickCount].x = x;
            clipWindowPoints[shapeClickCount].y = y;
            shapeClickCount++;
            if (shapeClickCount < 4) return 0;
            // After 4 points, set window
            int algoSel = (int)SendMessageW(hComboAlgo, CB_GETCURSEL, 0, 0);
            clipWindowIsSquare = (algoSel == 1); // 0: Rectangle, 1: Square
            clipWindowSet = true;
            UpdateClipWindowBounds();
            // Draw window border in a visible color (e.g., red)
            Line line(hMemDC);
            line.DrawLineDDA(clipMinX, clipMinY, clipMaxX, clipMinY, RGB(255,0,0));
            line.DrawLineDDA(clipMaxX, clipMinY, clipMaxX, clipMaxY, RGB(255,0,0));
            line.DrawLineDDA(clipMaxX, clipMaxY, clipMinX, clipMaxY, RGB(255,0,0));
            line.DrawLineDDA(clipMinX, clipMaxY, clipMinX, clipMinY, RGB(255,0,0));
            InvalidateRect(hWnd, NULL, FALSE);
            shapeClickCount = 0;
            return 0;
        }
        if (currentShape == SHAPE_ELLIPSE) {
            shapePoints[shapeClickCount].x = x;
            shapePoints[shapeClickCount].y = y;
            shapeClickCount++;
            if (shapeClickCount < 3) { return 0; }
            int xc = shapePoints[0].x, yc = shapePoints[0].y;
            int a = abs(shapePoints[1].x - xc);
            int b = abs(shapePoints[2].y - yc);
            int algoSel = (int)SendMessageW(hComboAlgo, CB_GETCURSEL, 0, 0);
            switch (algoSel) {
            case 0: DrawEllipseDirect(hMemDC, xc, yc, a, b, g_LineColor); break;
            case 1: DrawEllipsePolar(hMemDC, xc, yc, a, b, g_LineColor); break;
            case 2: DrawEllipseMidpoint(hMemDC, xc, yc, a, b, g_LineColor); break;
            }
            InvalidateRect(hWnd, NULL, FALSE);
            shapeClickCount = 0;
            return 0;
        }
        if (!waitingForSecondClick) {
            lineStart.x = x;
            lineStart.y = y;
            waitingForSecondClick = true;
        }
        else {
            lineEnd.x = x;
            lineEnd.y = y;
            int algoSel = (int)SendMessageW(hComboAlgo, CB_GETCURSEL, 0, 0);
            if (currentShape == SHAPE_LINE) {
                Line line(hMemDC);
                switch (algoSel) {
                case 0: line.DrawLineDDA(lineStart.x, lineStart.y, lineEnd.x, lineEnd.y, g_LineColor); break;
                case 1: line.DrawLineMidpoint(lineStart.x, lineStart.y, lineEnd.x, lineEnd.y, g_LineColor); break;
                case 2: line.DrawLineParametric(lineStart.x, lineStart.y, lineEnd.x, lineEnd.y, g_LineColor); break;
                }
            } else if (currentShape == SHAPE_CIRCLE) {
                int xc = lineStart.x, yc = lineStart.y;
                int R = (int)round(sqrt((lineEnd.x - xc) * (lineEnd.x - xc) + (lineEnd.y - yc) * (lineEnd.y - yc)));
                Circle circle(hMemDC);
                switch (algoSel) {
                case 0: circle.DrawCircleDirect(xc, yc, R, g_LineColor); break;
                case 1: circle.DrawCirclePolar(xc, yc, R, g_LineColor); break;
                case 2: circle.DrawCircleIterativePolar(xc, yc, R, g_LineColor); break;
                case 3: circle.DrawCircleMidpoint(xc, yc, R, g_LineColor); break;
                case 4: circle.DrawCircleModifiedMidpoint(xc, yc, R, g_LineColor); break;
                }
            } else if (currentShape == SHAPE_CIRCLE_QUARTER) {
                int xc = lineStart.x, yc = lineStart.y;
                int R = (int)round(sqrt((lineEnd.x - xc) * (lineEnd.x - xc) + (lineEnd.y - yc) * (lineEnd.y - yc)));
                Circle circle(hMemDC);
                // Always draw the full circle first
                circle.DrawCircleModifiedMidpoint(xc, yc, R, g_LineColor);
                switch (algoSel) {
                case 0: circle.FillQuarterWithCircles(xc, yc, R, 1); break;
                case 1: circle.FillQuarterWithCircles(xc, yc, R, 2); break;
                case 2: circle.FillQuarterWithCircles(xc, yc, R, 3); break;
                case 3: circle.FillQuarterWithCircles(xc, yc, R, 4); break;
                case 4: circle.FillQuarterWithLines(xc, yc, R, g_LineColor, 1); break;
                case 5: circle.FillQuarterWithLines(xc, yc, R, g_LineColor, 2); break;
                case 6: circle.FillQuarterWithLines(xc, yc, R, g_LineColor, 3); break;
                case 7: circle.FillQuarterWithLines(xc, yc, R, g_LineColor, 4); break;
                }
            }
            InvalidateRect(hWnd, NULL, FALSE);
            waitingForSecondClick = false;
        }
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT rc = { 0, 0, canvasWidth, topOffset };
        FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));
        BitBlt(hdc, 0, topOffset, canvasWidth, canvasHeight, hMemDC, 0, 0, SRCCOPY);
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY:
        if (hBitmap)
            DeleteObject(hBitmap);
        if (hMemDC)
            DeleteDC(hMemDC);
        PostQuitMessage(0);
        break;

    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT) {
            SetCursor(LoadCursor(NULL, IDC_CROSS));
            return TRUE;
        }
        break;

    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* mmi = (MINMAXINFO*)lParam;
        mmi->ptMinTrackSize.x = canvasWidth + 16;
        mmi->ptMinTrackSize.y = canvasHeight + 39;
        mmi->ptMaxTrackSize.x = canvasWidth + 16;
        mmi->ptMaxTrackSize.y = canvasHeight + 39;
        return 0;
    }

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
