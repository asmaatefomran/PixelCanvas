#ifndef FILLING_H
#define FILLING_H

#include <windows.h>
#include <vector>
#include <queue>

// Point structure used for filling operations
struct point {
    double x, y;
    point(double x = 0, double y = 0) : x(x), y(y) {}
};

// Function declarations for drawing and filling
void DrawLineDDA(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c);
void fillGeneralPolygon(HDC hdc, point p[], int n, COLORREF c);
void convexfill(HDC hdc, point p[], int n, COLORREF c);
void myFloodFill(HDC hdc, int x, int y, COLORREF bc, COLORREF fc);
void myFloodFillQueue(HDC hdc, int x, int y, COLORREF bc, COLORREF fc);
bool IsPolygonConvex(const std::vector<point>& points);

// Helper function to round a double to an int
inline int Round(double x) {
    return (int)(x + 0.5);
}

#endif // FILLING_H 