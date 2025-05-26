#include "Circle.h"
#include <cmath>
#include <cstdlib>

Circle::Circle(HDC hdc) : hdc(hdc) {}

void Circle::Draw8Points(int xc, int yc, int x, int y, COLORREF c) {
    SetPixel(hdc, xc + x, yc + y, c);
    SetPixel(hdc, xc - x, yc + y, c);
    SetPixel(hdc, xc + x, yc - y, c);
    SetPixel(hdc, xc - x, yc - y, c);
    SetPixel(hdc, xc + y, yc + x, c);
    SetPixel(hdc, xc - y, yc + x, c);
    SetPixel(hdc, xc + y, yc - x, c);
    SetPixel(hdc, xc - y, yc - x, c);
}


void Circle::DrawCircleDirect(int xc, int yc, int R, COLORREF c) {
    for (int x = 0; x <= R / sqrt(2); ++x) {
        int y = static_cast<int>(round(sqrt(R * R - x * x)));
        Draw8Points(xc, yc, x, y, c);
    }
}

void Circle::DrawCirclePolar(int xc, int yc, int R, COLORREF c) {
    double theta = 0;
    double inc = 1.0 / R;
    while (theta <= M_PI / 4) {
        int x = static_cast<int>(round(R * cos(theta)));
        int y = static_cast<int>(round(R * sin(theta)));
        Draw8Points(xc, yc, x, y, c);
        theta += inc;
    }
}

void Circle::DrawCircleIterativePolar(int xc, int yc, int R, COLORREF c) {
    double theta = 0;
    double cosInc = cos(1.0 / R);
    double sinInc = sin(1.0 / R);
    double x = R, y = 0;

    while (x >= y) {
        Draw8Points(xc, yc, static_cast<int>(round(x)), static_cast<int>(round(y)), c);
        double xx = x * cosInc - y * sinInc;
        y = x * sinInc + y * cosInc;
        x = xx;
    }
}

void Circle::DrawCircleMidpoint(int xc, int yc, int R, COLORREF c) {
    int x = 0, y = R;
    int d = 1 - R;

    Draw8Points(xc, yc, x, y, c);
    while (x < y) {
        if (d < 0) {
            d += 2 * x + 3;
        } else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
        Draw8Points(xc, yc, x, y, c);
    }
}

void Circle::DrawCircleModifiedMidpoint(int xc, int yc, int R, COLORREF c) {
   int x = 0, y = R;
    int d = 1 - R;
    int d1 = 3, d2 = 5 - 2 * R;

    Draw8Points(xc, yc, x, y, c);

    while (x < y) {
        if (d < 0) {
            d += d1;
            d1 += 2;
            d2 += 2;
            x++;
        } else {
            d += d2;
            d1 += 2;
            d2 += 4;
            x++;
            y--;
        }
        Draw8Points(xc, yc, x, y, c);
    }
}

void Circle::FillWithCircles(int xc, int yc, int R) {
    int dec = R / 100;
    while (R > 0) {
        R -= dec;
        COLORREF randomColor = RGB(rand() % 256, rand() % 256, rand() % 256);
        DrawCircleModifiedMidpoint(xc, yc, R, randomColor);
    }
}
