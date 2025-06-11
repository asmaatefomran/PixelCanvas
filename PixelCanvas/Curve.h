#ifndef CURVE_H
#define CURVE_H

#include <windows.h>

class Curve {
public:
    Curve(HDC hdc);
    void FillWithHermite(int x1, int y1, int x2, int y2, COLORREF color);
    void DrawBezier(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, COLORREF color);
    void FillWithBezier(int x1, int y1, int x2, int y2, COLORREF color);
    void DrawCardinalSpline(POINT* pts, int n, double c, COLORREF color);
    void DrawHermite(int x0, int y0, int x1, int y1, int t0, int t1, COLORREF color);

private:
    HDC hdc;
};

#endif // CURVE_H 