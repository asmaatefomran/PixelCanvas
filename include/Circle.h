#ifndef CIRCLE_H
#define CIRCLE_H

#include <windows.h>
#include "Line.h"

class Circle {
public:
    Circle(HDC hdc);
    void DrawCircleDirect(int xc, int yc, int R, COLORREF c);  //needs click up and down
    void DrawCirclePolar(int xc, int yc, int R, COLORREF c);   //needs click up and down
    void DrawCircleIterativePolar(int xc, int yc, int R, COLORREF c); //needs click up and down
    void DrawCircleMidpoint(int xc, int yc, int R, COLORREF c);  //needs click up and down
    void DrawCircleModifiedMidpoint(int xc, int yc, int R, COLORREF c); //needs click up and down
    void FillWithCircles(int xc, int yc, int R); //needs click up and down
    void FillQuarter(int xc, int yc, int R, COLORREF c, int quarter); // needs click up and down  +  the number of wanted quarter to file 
    

private:
    void Draw8Points(int xc, int yc, int x, int y, COLORREF c);
    void Draw2Lines(int xc, int yc, int x, int y, COLORREF c, int quarter);

    HDC hdc;
    Line line; 
};

#endif // CIRCLE_H
