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
    void FillQuarterWithCircles(int xc, int yc, int R, int quarter); // needs click up and down  +  the number of wanted quarter to file
    void FillWithCircles(int xc, int yc, int R); //needs click up and down
    void FillQuarterWithLines(int xc, int yc, int R, COLORREF c, int quarter); // needs click up and down  +  the number of wanted quarter to file
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
