#ifndef LINE_H
#define LINE_H

#include <windows.h>
#include <cmath>
#include <algorithm>

class Line {
public:
    Line(HDC hdc);

    void DrawLineDDA(int x1, int y1, int x2, int y2, COLORREF c);
    void DrawLineMidpoint(int x1, int y1, int x2, int y2, COLORREF c);
    void DrawLineParametric(int x1, int y1, int x2, int y2, COLORREF c);
    void DrawLineInterpolated(int x1, int y1, int x2, int y2, COLORREF c1, COLORREF c2);

private:
    COLORREF Mix(COLORREF c1, COLORREF c2, float t);
    HDC hdc;
};

#endif 