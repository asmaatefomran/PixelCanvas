#ifndef CIRCLE_H
#define CIRCLE_H

#include <windows.h>
#include "Line.h"

class Circle {
public:
    Circle(HDC hdc);
    void DrawCircleDirect(int xc, int yc, int R, COLORREF c);
    void DrawCirclePolar(int xc, int yc, int R, COLORREF c);
    void DrawCircleIterativePolar(int xc, int yc, int R, COLORREF c);
    void DrawCircleMidpoint(int xc, int yc, int R, COLORREF c);
    void DrawCircleModifiedMidpoint(int xc, int yc, int R, COLORREF c);
    void FillQuarterWithCircles(int xc, int yc, int R, int quarter);
    void FillWithCircles(int xc, int yc, int R);
    void FillQuarterWithLines(int xc, int yc, int R, COLORREF c, int quarter);
    void FillWithLines(int xc, int yc, int R, COLORREF c);

private:
    void Draw8Points(int xc, int yc, int x, int y, COLORREF c);
    void Draw2Points(int xc, int yc, int x, int y, COLORREF c, int quarter);
    void Draw2Lines(int xc, int yc, int x, int y, COLORREF c, int quarter);
    void Draw8Lines(int xc, int yc, int x, int y, COLORREF c);
    void DrawQuarterCircleModifiedMidpoint(int xc, int yc, int R, COLORREF c,int quarter);

    HDC hdc;
    Line line;
};

#endif // CIRCLE_H 