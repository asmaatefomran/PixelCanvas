#include "Curve.h"
#include <cmath>
#include <algorithm>
#include <vector>
#include "PixelCanvas.h"
using namespace std;

extern bool clipWindowSet;
extern int clipMinX, clipMaxX, clipMinY, clipMaxY;

struct Point {
    double x, y;
};
struct Derivative {
    double u, v;
};

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
    int left = min(x1, x2);
    int right = max(x1, x2);
    int top = min(y1, y2);
    int bottom = max(y1, y2);
    for (int x = left; x <= right; x++) {
        DrawHermite2((double)x, (double)top, (double)x, (double)bottom, 0.0, 1.0, 0.0, -1.0, color);
    }
}

void Curve::DrawHermite2(double x0, double y0, double x1, double y1, double t0x, double t0y, double t1x, double t1y, COLORREF color) {
    double h00, h10, h01, h11;
    for (double t = 0; t <= 1; t += 0.001) {
        h00 = 2 * t * t * t - 3 * t * t + 1;
        h10 = t * t * t - 2 * t * t + t;
        h01 = -2 * t * t * t + 3 * t * t;
        h11 = t * t * t - t * t;
        double x = h00 * x0 + h10 * t0x + h01 * x1 + h11 * t1x;
        double y = h00 * y0 + h10 * t0y + h01 * y1 + h11 * t1y;
        if (!clipWindowSet || (x >= clipMinX && x <= clipMaxX && y >= clipMinY && y <= clipMaxY))
            SetPixel(hdc, round(x), round(y), color);
    }
}
void Curve::DrawCardinalSpline(POINT* pts, int n, double c, COLORREF color) {
    if (n < 4) return;
    struct Vector2 { double x, y; };
    double c1 = 1 - c;
    std::vector<Vector2> P;
    for (int i = 0; i < n; ++i) P.push_back(Vector2{(double)pts[i].x, (double)pts[i].y});
    Vector2 T0;
    T0.x = c1 * (P[2].x - P[0].x);
    T0.y = c1 * (P[2].y - P[0].y);
    for (int i = 2; i < n - 1; i++) {
        Vector2 T1;
        T1.x = c1 * (P[i + 1].x - P[i - 1].x);
        T1.y = c1 * (P[i + 1].y - P[i - 1].y);
        DrawHermite2(P[i - 1].x, P[i - 1].y, P[i].x, P[i].y, T0.x, T0.y, T1.x, T1.y, color);
        T0 = T1;
    }
}


void Curve::DrawBezier(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, COLORREF color) {
        for (double t = 0; t <= 1; t += 0.001) {
            double mt = 1 - t;
            int x = (int)(pow(mt, 3) * x0 + 3 * pow(mt, 2) * t * x1 +
                3 * mt * pow(t, 2) * x2 + pow(t, 3) * x3);
            int y = (int)(pow(mt, 3) * y0 + 3 * pow(mt, 2) * t * y1 +
                3 * mt * pow(t, 2) * y2 + pow(t, 3) * y3);
            if (!clipWindowSet || (x >= clipMinX && x <= clipMaxX && y >= clipMinY && y <= clipMaxY))
            SetPixel(hdc, x, y, color);
        }
}

void Curve::FillWithBezier(int x1, int y1, int x2, int y2, COLORREF color) {
    int left = min(x1, x2);
    int right = max(x1, x2);
    int top = min(y1, y2);
    int bottom = max(y1, y2);
    for (int y = top; y <= bottom; y++) {
        DrawBezier(left, y,
                   left + (right - left) / 3, y,
                   right - (right - left) / 3, y,
                   right, y,
                   color);
    }
}