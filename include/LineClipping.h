#include <windows.h>
#include <cmath>
#include <stdio.h>

struct Point {
    double x, y;
};

Point in[2]; // Input points for clipping window
const int polygonSize = 2;
Point polygon[polygonSize]; // Line endpoints
int cnt = 0, cnt2 = 0;
double xleft, xright, ytop, ybottom;

int Round(double x) {
    return (int)(x + 0.5);
}

union Code {
    unsigned all : 4;
    struct {
        unsigned left : 1;
        unsigned right : 1;
        unsigned bottom : 1;
        unsigned top : 1;
    };
};

Code getCode(double x, double y) {
    Code code = { 0 };
    if (x < xleft) code.left = 1;
    else if (x > xright) code.right = 1;
    if (y < ytop) code.top = 1;
    else if (y > ybottom) code.bottom = 1;
    return code;
}

void verticalIntersection(double xs, double ys, double xe, double ye, int x, double* xi, double* yi) {
    *xi = x;
    *yi = ys + (ye - ys) * (x - xs) / (xe - xs);
}

void horizontalIntersection(double xs, double ys, double xe, double ye, int y, double* xi, double* yi) {
    *yi = y;
    *xi = xs + (y - ys) * (xe - xs) / (ye - ys);
}

void DrawLineMidpoint(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color = RGB(0, 255, 0)) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);

    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;

    int x = x1, y = y1;

    if (dy <= dx) {
        int d = 2 * dy - dx;
        int incrE = 2 * dy;
        int incrNE = 2 * (dy - dx);
        SetPixel(hdc, x, y, color);
        for (int i = 0; i < dx; ++i) {
            if (d <= 0) {
                d += incrE;
                x += sx;
            }
            else {
                d += incrNE;
                x += sx;
                y += sy;
            }
            SetPixel(hdc, x, y, color);
        }
    }
    else {
        int d = 2 * dx - dy;
        int incrN = 2 * dx;
        int incrNE = 2 * (dx - dy);
        SetPixel(hdc, x, y, color);
        for (int i = 0; i < dy; ++i) {
            if (d <= 0) {
                d += incrN;
                y += sy;
            }
            else {
                d += incrNE;
                x += sx;
                y += sy;
            }
            SetPixel(hdc, x, y, color);
        }
    }
}

void CohenSuth(HDC hdc, double xs, double ys, double xe, double ye) {
    double x1 = xs, y1 = ys, x2 = xe, y2 = ye;
    Code c1 = getCode(x1, y1);
    Code c2 = getCode(x2, y2);

    while ((c1.all || c2.all) && !(c1.all & c2.all)) {
        double xi, yi;
        if (c1.all) {
            if (c1.left) verticalIntersection(x1, y1, x2, y2, xleft, &xi, &yi);
            else if (c1.right) verticalIntersection(x1, y1, x2, y2, xright, &xi, &yi);
            else if (c1.top) horizontalIntersection(x1, y1, x2, y2, ytop, &xi, &yi);
            else if (c1.bottom) horizontalIntersection(x1, y1, x2, y2, ybottom, &xi, &yi);
            x1 = xi;
            y1 = yi;
            c1 = getCode(x1, y1);
        }
        else {
            if (c2.left) verticalIntersection(x1, y1, x2, y2, xleft, &xi, &yi);
            else if (c2.right) verticalIntersection(x1, y1, x2, y2, xright, &xi, &yi);
            else if (c2.top) horizontalIntersection(x1, y1, x2, y2, ytop, &xi, &yi);
            else if (c2.bottom) horizontalIntersection(x1, y1, x2, y2, ybottom, &xi, &yi);
            x2 = xi;
            y2 = yi;
            c2 = getCode(x2, y2);
        }
    }

    if (!(c1.all | c2.all)) {
        DrawLineMidpoint(hdc, Round(x1), Round(y1), Round(x2), Round(y2));
    }
}

void PointClipping(HWND hwnd, Point p) {
    if (p.x >= xleft && p.x <= xright && p.y >= ytop && p.y <= ybottom) {
        HDC hdc = GetDC(hwnd);
        SetPixel(hdc, p.x, p.y, RGB(255, 0, 0));
        ReleaseDC(hwnd, hdc);
    }

    printf("Clicked point: (%d, %d)\n", (int)p.x, (int)p.y);
    printf("xleft: %d, xright: %d, ytop: %d, ybottom: %d\n", (int)xleft, (int)xright, (int)ytop, (int)ybottom);
}

void getSquareWindow() {
    xleft = min(in[0].x, in[1].x);
    xright = max(in[0].x, in[1].x);
    int side = abs(in[0].x - in[1].x);
    if (in[1].y < in[0].y) {
        ytop = in[0].y - side;
        ybottom = in[0].y;
    }
    else {
        ytop = in[0].y;
        ybottom = in[0].y + side;
    }
}

void drawWindowBorder(HWND hwnd) {
    HDC hdc = GetDC(hwnd);
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
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

// LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
//     switch (msg) {
//     case WM_RBUTTONDOWN:
//         if (cnt == 0) {
//             in[0].x = LOWORD(lParam);
//             in[0].y = HIWORD(lParam);
//             cnt++;
//         }
//         else if (cnt == 1) {
//             in[1].x = LOWORD(lParam);
//             in[1].y = HIWORD(lParam);
//             cnt++;
//             getSquareWindow();
//             drawWindowBorder(hwnd);
//         }
//         break;

//     case WM_LBUTTONDOWN:
//         if (cnt2 < polygonSize) {
//             polygon[cnt2].x = LOWORD(lParam);
//             polygon[cnt2].y = HIWORD(lParam);
//             cnt2++;
//         }
//         if (cnt2 == polygonSize) {
//             HDC hdc = GetDC(hwnd);
//             CohenSuth(hdc, polygon[0].x, polygon[0].y, polygon[1].x, polygon[1].y);
//             ReleaseDC(hwnd, hdc);
//             cnt2 = 0;
//         }
//         break;

//     case WM_DESTROY:
//         PostQuitMessage(0);
//         break;

//     default:
//         return DefWindowProc(hwnd, msg, wParam, lParam);
//     }
//     return 0;
// }

// int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
//     LPSTR lpCmdLine, int nCmdShow) {

//     WNDCLASS wc = {};
//     wc.lpfnWndProc = WndProc;
//     wc.hInstance = hInstance;
//     wc.lpszClassName = L"LineClippingClass";
//     wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
//     wc.hCursor = LoadCursor(NULL, IDC_ARROW);
//     wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
//     RegisterClass(&wc);

//     HWND hwnd = CreateWindow(L"LineClippingClass", L"Cohen-Sutherland Line Clipping",
//         WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
//         800, 600, NULL, NULL, hInstance, NULL);

//     ShowWindow(hwnd, nCmdShow);
//     UpdateWindow(hwnd);

//     MSG msg;
//     while (GetMessage(&msg, NULL, 0, 0)) {
//         TranslateMessage(&msg);
//         DispatchMessage(&msg);
//     }
//     return (int)msg.wParam;
// }
