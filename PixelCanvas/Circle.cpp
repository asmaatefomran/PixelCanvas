#include "Circle.h"
#include <cmath>
#include <cstdlib>
#include "PixelCanvas.h"

extern bool clipWindowSet;
extern int clipMinX, clipMaxX, clipMinY, clipMaxY;

Circle::Circle(HDC hdc) : hdc(hdc), line(hdc) {}

void Circle::Draw8Points(int xc, int yc, int x, int y, COLORREF c) {
    if (!clipWindowSet || (xc + x >= clipMinX && xc + x <= clipMaxX && yc + y >= clipMinY && yc + y <= clipMaxY)) SetPixel(hdc, xc + x, yc + y, c);
    if (!clipWindowSet || (xc - x >= clipMinX && xc - x <= clipMaxX && yc + y >= clipMinY && yc + y <= clipMaxY)) SetPixel(hdc, xc - x, yc + y, c);
    if (!clipWindowSet || (xc + x >= clipMinX && xc + x <= clipMaxX && yc - y >= clipMinY && yc - y <= clipMaxY)) SetPixel(hdc, xc + x, yc - y, c);
    if (!clipWindowSet || (xc - x >= clipMinX && xc - x <= clipMaxX && yc - y >= clipMinY && yc - y <= clipMaxY)) SetPixel(hdc, xc - x, yc - y, c);
    if (!clipWindowSet || (xc + y >= clipMinX && xc + y <= clipMaxX && yc + x >= clipMinY && yc + x <= clipMaxY)) SetPixel(hdc, xc + y, yc + x, c);
    if (!clipWindowSet || (xc - y >= clipMinX && xc - y <= clipMaxX && yc + x >= clipMinY && yc + x <= clipMaxY)) SetPixel(hdc, xc - y, yc + x, c);
    if (!clipWindowSet || (xc + y >= clipMinX && xc + y <= clipMaxX && yc - x >= clipMinY && yc - x <= clipMaxY)) SetPixel(hdc, xc + y, yc - x, c);
    if (!clipWindowSet || (xc - y >= clipMinX && xc - y <= clipMaxX && yc - x >= clipMinY && yc - x <= clipMaxY)) SetPixel(hdc, xc - y, yc - x, c);
}
void Circle::Draw2Points(int xc, int yc, int x, int y, COLORREF c, int quarter) {
    switch (quarter) {
        case 1:  // Top-right
            if (!clipWindowSet || (xc + x >= clipMinX && xc + x <= clipMaxX && yc - y >= clipMinY && yc - y <= clipMaxY)) SetPixel(hdc, xc + x, yc - y, c);
            if (!clipWindowSet || (xc + y >= clipMinX && xc + y <= clipMaxX && yc - x >= clipMinY && yc - x <= clipMaxY)) SetPixel(hdc, xc + y, yc - x, c);
            break;
        case 2:  // Top-left
            if (!clipWindowSet || (xc - x >= clipMinX && xc - x <= clipMaxX && yc - y >= clipMinY && yc - y <= clipMaxY)) SetPixel(hdc, xc - x, yc - y, c);
            if (!clipWindowSet || (xc - y >= clipMinX && xc - y <= clipMaxX && yc - x >= clipMinY && yc - x <= clipMaxY)) SetPixel(hdc, xc - y, yc - x, c);
            break;
        case 3:  // Bottom-left
            if (!clipWindowSet || (xc - x >= clipMinX && xc - x <= clipMaxX && yc + y >= clipMinY && yc + y <= clipMaxY)) SetPixel(hdc, xc - x, yc + y, c);
            if (!clipWindowSet || (xc - y >= clipMinX && xc - y <= clipMaxX && yc + x >= clipMinY && yc + x <= clipMaxY)) SetPixel(hdc, xc - y, yc + x, c);
            break;
        case 4:  // Bottom-right
            if (!clipWindowSet || (xc + x >= clipMinX && xc + x <= clipMaxX && yc + y >= clipMinY && yc + y <= clipMaxY)) SetPixel(hdc, xc + x, yc + y, c);
            if (!clipWindowSet || (xc + y >= clipMinX && xc + y <= clipMaxX && yc + x >= clipMinY && yc + x <= clipMaxY)) SetPixel(hdc, xc + y, yc + x, c);
            break;
    }
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
    while (theta <= 3.14 / 4) {
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
void Circle::DrawQuarterCircleModifiedMidpoint(int xc, int yc, int R, COLORREF c,int quarter) {
    int x = 0, y = R;
    int d = 1 - R;
    int d1 = 3, d2 = 5 - 2 * R;

    Draw2Points(xc, yc, x, y, c,quarter);

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
        Draw2Points(xc, yc, x, y, c,quarter);
    }
}
void Circle::FillQuarterWithCircles(int xc, int yc, int R, int quarter) {
    int dec = R / 100;
    if (dec == 0) dec = 1;
    while (R > 0) {
        R -= dec;
        COLORREF randomColor = RGB(rand() % 256, rand() % 256, rand() % 256);
        DrawQuarterCircleModifiedMidpoint( xc, yc,  R, randomColor, quarter);
    }
}
void Circle::FillWithCircles(int xc, int yc, int R) {
    int dec = R / 100;
    if (dec == 0) dec = 1;
    while (R > 0) {
        R -= dec;
        COLORREF randomColor = RGB(rand() % 256, rand() % 256, rand() % 256);
        DrawCircleModifiedMidpoint(xc, yc, R, randomColor);
    }
}

void Circle::Draw2Lines(int xc, int yc, int x, int y, COLORREF c, int quarter) {
    // For all quarters, draw lines from center to arc points
    switch (quarter) {
    case 1: // Top-right
        line.DrawLineDDA(xc, yc, xc + x, yc - y, c);
        line.DrawLineDDA(xc, yc, xc + y, yc - x, c);
        break;
    case 2: // Top-left
        line.DrawLineDDA(xc, yc, xc - x, yc - y, c);
        line.DrawLineDDA(xc, yc, xc - y, yc - x, c);
        break;
    case 3: // Bottom-left
        line.DrawLineDDA(xc, yc, xc - x, yc + y, c);
        line.DrawLineDDA(xc, yc, xc - y, yc + x, c);
        break;
    case 4: // Bottom-right
        line.DrawLineDDA(xc, yc, xc + x, yc + y, c);
        line.DrawLineDDA(xc, yc, xc + y, yc + x, c);
        break;
    }
}
void Circle::Draw8Lines(int xc, int yc, int x, int y, COLORREF c) {
    line.DrawLineDDA(max(clipWindowSet ? clipMinX : INT_MIN, min(xc, xc + x)), max(clipWindowSet ? clipMinY : INT_MIN, min(yc, yc - y)), min(clipWindowSet ? clipMaxX : INT_MAX, max(xc, xc + x)), min(clipWindowSet ? clipMaxY : INT_MAX, max(yc, yc - y)), c);
    line.DrawLineDDA(max(clipWindowSet ? clipMinX : INT_MIN, min(xc, xc + y)), max(clipWindowSet ? clipMinY : INT_MIN, min(yc, yc - x)), min(clipWindowSet ? clipMaxX : INT_MAX, max(xc, xc + y)), min(clipWindowSet ? clipMaxY : INT_MAX, max(yc, yc - x)), c);
    line.DrawLineDDA(max(clipWindowSet ? clipMinX : INT_MIN, min(xc, xc - x)), max(clipWindowSet ? clipMinY : INT_MIN, min(yc, yc - y)), min(clipWindowSet ? clipMaxX : INT_MAX, max(xc, xc - x)), min(clipWindowSet ? clipMaxY : INT_MAX, max(yc, yc - y)), c);
    line.DrawLineDDA(max(clipWindowSet ? clipMinX : INT_MIN, min(xc, xc - y)), max(clipWindowSet ? clipMinY : INT_MIN, min(yc, yc - x)), min(clipWindowSet ? clipMaxX : INT_MAX, max(xc, xc - y)), min(clipWindowSet ? clipMaxY : INT_MAX, max(yc, yc - x)), c);
    line.DrawLineDDA(max(clipWindowSet ? clipMinX : INT_MIN, min(xc, xc - x)), max(clipWindowSet ? clipMinY : INT_MIN, min(yc, yc + y)), min(clipWindowSet ? clipMaxX : INT_MAX, max(xc, xc - x)), min(clipWindowSet ? clipMaxY : INT_MAX, max(yc, yc + y)), c);
    line.DrawLineDDA(max(clipWindowSet ? clipMinX : INT_MIN, min(xc, xc - y)), max(clipWindowSet ? clipMinY : INT_MIN, min(yc, yc + x)), min(clipWindowSet ? clipMaxX : INT_MAX, max(xc, xc - y)), min(clipWindowSet ? clipMaxY : INT_MAX, max(yc, yc + x)), c);
    line.DrawLineDDA(max(clipWindowSet ? clipMinX : INT_MIN, min(xc, xc + x)), max(clipWindowSet ? clipMinY : INT_MIN, min(yc, yc + y)), min(clipWindowSet ? clipMaxX : INT_MAX, max(xc, xc + x)), min(clipWindowSet ? clipMaxY : INT_MAX, max(yc, yc + y)), c);
    line.DrawLineDDA(max(clipWindowSet ? clipMinX : INT_MIN, min(xc, xc + y)), max(clipWindowSet ? clipMinY : INT_MIN, min(yc, yc + x)), min(clipWindowSet ? clipMaxX : INT_MAX, max(xc, xc + y)), min(clipWindowSet ? clipMaxY : INT_MAX, max(yc, yc + x)), c);
}
void Circle::FillQuarterWithLines(int xc, int yc, int R, COLORREF c, int quarter) {
    int x = 0, y = R;
    int d = 1 - R;
    int d1 = 3, d2 = 5 - 2 * R;
    DrawCircleModifiedMidpoint(xc, yc, R, c);
    Draw2Lines(xc, yc, x, y, c, quarter);
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
        Draw2Lines(xc, yc, x, y, c, quarter);
    }
}
void Circle::FillWithLines(int xc, int yc, int R, COLORREF c) {
    // Draw the circle outline
    DrawCircleModifiedMidpoint(xc, yc, R, c);
    // Fill with radial lines
    const int numSteps = 360; // 1-degree steps
    for (int i = 0; i < numSteps; ++i) {
        double theta = 2 * 3.14 * i / numSteps;
        int x = static_cast<int>(round(xc + R * cos(theta)));
        int y = static_cast<int>(round(yc + R * sin(theta)));
        line.DrawLineDDA(xc, yc, x, y, c);
    }
} 