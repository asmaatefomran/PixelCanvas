#ifndef POLYGONFILL_H
#define POLYGONFILL_H

#include <windows.h>
#include <queue>
#include <climits>
#include <algorithm>

struct point {
    double x, y;
    point(double x = 0, double y = 0) : x(x), y(y) {}
};

void DrawLineDDA(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c);
void fillGeneralPolygon(HDC hdc, point p[], int n, COLORREF c);
void convexfill(HDC hdc, point p[], int n, COLORREF c);
void myFloodFill(HDC hdc, int x, int y, COLORREF bc, COLORREF fc);
void myFloodFillqueue(HDC hdc, int x, int y, COLORREF bc, COLORREF fc);

#endif // POLYGONFILL_H 