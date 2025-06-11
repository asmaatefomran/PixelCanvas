#include "Ellipse.h"
#include <cmath>
#include "PixelCanvas.h"

extern bool clipWindowSet;
extern int clipMinX, clipMaxX, clipMinY, clipMaxY;

static void Draw4Points(HDC hdc, int xc, int yc, int x, int y, COLORREF color) {
    if (!clipWindowSet || (xc + x >= clipMinX && xc + x <= clipMaxX && yc + y >= clipMinY && yc + y <= clipMaxY)) SetPixel(hdc, xc + x, yc + y, color);
    if (!clipWindowSet || (xc - x >= clipMinX && xc - x <= clipMaxX && yc + y >= clipMinY && yc + y <= clipMaxY)) SetPixel(hdc, xc - x, yc + y, color);
    if (!clipWindowSet || (xc + x >= clipMinX && xc + x <= clipMaxX && yc - y >= clipMinY && yc - y <= clipMaxY)) SetPixel(hdc, xc + x, yc - y, color);
    if (!clipWindowSet || (xc - x >= clipMinX && xc - x <= clipMaxX && yc - y >= clipMinY && yc - y <= clipMaxY)) SetPixel(hdc, xc - x, yc - y, color);
}

void DrawEllipseDirect(HDC hdc, int xc, int yc, int a, int b, COLORREF color) {
    int a2 = a * a;
    int b2 = b * b;
    for (int x = 0; x <= a; ++x) {
        double y = b * sqrt(1.0 - (double)x * x / a2);
        Draw4Points(hdc, xc, yc, x, round(y), color);
    }
    for (int y = 0; y <= b; ++y) {
        double x = a * sqrt(1.0 - (double)y * y / b2);
        Draw4Points(hdc, xc, yc, round(x), y, color);
    }
}

void DrawEllipsePolar(HDC hdc, int xc, int yc, int a, int b, COLORREF color) {
    double PI = 3.14159265358979323846;
    for (double theta = 0; theta < 2 * PI; theta += 0.0005) {
        int x = round(a * cos(theta));
        int y = round(b * sin(theta));
        if (!clipWindowSet || (xc + x >= clipMinX && xc + x <= clipMaxX && yc + y >= clipMinY && yc + y <= clipMaxY))
            SetPixel(hdc, xc + x, yc + y, color);
    }
}

void DrawEllipseMidpoint(HDC hdc, int xc, int yc, int a, int b, COLORREF color) {
    int x = 0, y = b;
    int a2 = a * a, b2 = b * b;
    int d = b2 - a2 * b + 0.25 * a2;
    int dx = 2 * b2 * x;
    int dy = 2 * a2 * y;
    // Region 1
    while (dx < dy) {
        Draw4Points(hdc, xc, yc, x, y, color);
        if (d < 0) {
            x++;
            dx += 2 * b2;
            d += dx + b2;
        } else {
            x++;
            y--;
            dx += 2 * b2;
            dy -= 2 * a2;
            d += dx - dy + b2;
        }
    }
    // Region 2
    d = b2 * (x + 0.5) * (x + 0.5) + a2 * (y - 1) * (y - 1) - a2 * b2;
    while (y >= 0) {
        Draw4Points(hdc, xc, yc, x, y, color);
        if (d > 0) {
            y--;
            dy -= 2 * a2;
            d += a2 - dy;
        } else {
            x++;
            y--;
            dx += 2 * b2;
            dy -= 2 * a2;
            d += dx - dy + a2;
        }
    }
} 