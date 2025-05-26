#include "Curve.h"
#include <cmath>

Curve::Curve(HDC hdc) : hdc(hdc) {}

void Curve::DrawHermite(int x0, int y0, int x1, int y1, int t0, int t1, COLORREF color) {
    for (double t = 0; t <= 1; t += 0.001) {
        double h1 = 2 * pow(t, 3) - 3 * pow(t, 2) + 1;
        double h2 = -2 * pow(t, 3) + 3 * pow(t, 2);
        double h3 = pow(t, 3) - 2 * pow(t, 2) + t;
        double h4 = pow(t, 3) - pow(t, 2);

        int x = (int)(h1 * x0 + h2 * x1 + h3 * t0 + h4 * t1);
        int y = (int)(h1 * y0 + h2 * y1);

        SetPixel(hdc, x, y, color);
    }
}

void Curve::FillWithHermite(int x1, int y1, int x2, int y2, COLORREF color) {
    int step = 2;
    for (int x = x1; x <= x2; x += step) {
        DrawHermite(x, y1, x, y2, 40, 40, color);
    }
}
void Curve::DrawBezier(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, COLORREF color) {
    for (double t = 0; t <= 1; t += 0.001) {
        double mt = 1 - t;
        int x = (int)(pow(mt, 3) * x0 + 3 * pow(mt, 2) * t * x1 +
                                 3 * mt * pow(t, 2) * x2 + pow(t, 3) * x3);
        int y = (int)(pow(mt, 3) * y0 + 3 * pow(mt, 2) * t * y1 +
                                 3 * mt * pow(t, 2) * y2 + pow(t, 3) * y3);
        SetPixel(hdc, x, y, color);
    }
}

void Curve::FillWithBezier(int x1, int y1, int x2, int y2, COLORREF color) {
    int step = 2;
    for (int y = y1; y <= y2; y += step) {
        DrawBezier(x1, y, x1 + 20, y - 20, x2 - 20, y + 20, x2, y, color);
    }
}