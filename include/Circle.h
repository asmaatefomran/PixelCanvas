#ifndef CIRCLE_H
#define CIRCLE_H

#include <windows.h>

class Circle {
public:
    Circle(HDC hdc);
    void DrawCircleDirect(int xc, int yc, int R, COLORREF c);  //needs 2 clicks
    void DrawCirclePolar(int xc, int yc, int R, COLORREF c);   //needs 2 clicks
    void DrawCircleIterativePolar(int xc, int yc, int R, COLORREF c); //needs 2 clicks
    void DrawCircleMidpoint(int xc, int yc, int R, COLORREF c);  //needs 2 clicks
    void DrawCircleModifiedMidpoint(int xc, int yc, int R, COLORREF c); //needs 2 clicks
    void FillWithCircles(int xc, int yc, int R); //needs 2 clicks 
    

private:
    void Draw8Points(int xc, int yc, int x, int y, COLORREF c);
    HDC hdc;
};

#endif // CIRCLE_H
