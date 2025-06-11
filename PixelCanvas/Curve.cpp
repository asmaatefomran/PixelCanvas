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
    double t0x = t0, t0y = t0;
    double t1x = t1, t1y = t1;
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

void DrawCardinalSpline(HDC hdc, vector<Point>& points, double c, COLORREF color) {
    if (points.size() < 4) return;
    Derivative t1 = { c * (points[2].x - points[0].x), c * (points[2].y - points[0].y) };
    for (int i = 2; i < points.size() - 1; ++i) {
        Derivative t2 = { c * (points[i+1].x - points[i].x), c * (points[i + 1].y - points[i].y)};
        Point p1 = points[i-1];
        Point p2 = points[i];
        Derivative d1 = t1;
        Derivative d2 = t2;
        for (double t = 0; t <= 1; t += 0.001) {
            double h00 = 2 * t * t * t - 3 * t * t + 1;
            double h10 = t * t * t - 2 * t * t + t;
            double h01 = -2 * t * t * t + 3 * t * t;
            double h11 = t * t * t - t * t;
            double x = h00 * p1.x + h10 * d1.u + h01 * p2.x + h11 * d2.u;
            double y = h00 * p1.y + h10 * d1.v + h01 * p2.y + h11 * d2.v;
            if (!clipWindowSet || (x >= clipMinX && x <= clipMaxX && y >= clipMinY && y <= clipMaxY))
                SetPixel(hdc, round(x), round(y), color);
        }
        t1 = t2;
    }
}

void Curve::FillWithHermite(int x1, int y1, int x2, int y2, COLORREF color) {
    int step = 1;
    for (int x = x1; x <= x2; x += step) {
        int yTop = y1;
        int yBottom = y2;
        if (yTop > yBottom) std::swap(yTop, yBottom);
        for (int y = yTop; y <= yBottom; ++y) {
            if (!clipWindowSet || (x >= clipMinX && x <= clipMaxX && y >= clipMinY && y <= clipMaxY))
                SetPixel(hdc, x, y, color);
        }
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
    int step = 1;
    for (int y = y1; y <= y2; y += step) {
        int xLeft = x1;
        int xRight = x2;
        if (xLeft > xRight) std::swap(xLeft, xRight);
        for (int x = xLeft; x <= xRight; ++x) {
            if (!clipWindowSet || (x >= clipMinX && x <= clipMaxX && y >= clipMinY && y <= clipMaxY))
                SetPixel(hdc, x, y, color);
        }
    }
} 