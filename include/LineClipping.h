#ifndef LINE_CLIPPING_H
#define LINE_CLIPPING_H

#include <windows.h>
#include <cmath>
#include <stdio.h>
#include <algorithm> // For std::min and std::max

struct ClipPoint {
    double x, y;
};

union Code {
    unsigned all : 4;
    struct {
        unsigned left : 1;
        unsigned right : 1;
        unsigned bottom : 1;
        unsigned top : 1;
    };
};

extern ClipPoint in[2]; // Input points for clipping window
extern const int polygonSize;
extern ClipPoint linePoints[2]; // Line endpoints
extern int cnt, cnt2;
extern double xleft, xright, ytop, ybottom;

int RoundClip(double x);
Code getCode(double x, double y);
void verticalIntersection(double xs, double ys, double xe, double ye, int x, double* xi, double* yi);
void horizontalIntersection(double xs, double ys, double xe, double ye, int y, double* xi, double* yi);
void DrawLineMidpoint(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color = RGB(0, 255, 0));
void CohenSuth(HDC hdc, double xs, double ys, double xe, double ye);
void PointClipping(HWND hwnd, ClipPoint p);
void getSquareWindow();
void drawWindowBorder(HWND hwnd);

#endif // LINE_CLIPPING_H
