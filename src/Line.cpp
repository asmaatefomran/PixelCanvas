#include "Line.h"
using namespace std;

Line::Line(HDC hdc) : hdc(hdc) {}

void Line::DrawLineDDA(int x1, int y1, int x2, int y2, COLORREF c) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = max(abs(dx), abs(dy));
    float xInc = dx / (float)steps;
    float yInc = dy / (float)steps;
    float x = x1;
    float y = y1;
    for (int i = 0; i <= steps; ++i) {
        SetPixel(hdc, round(x), round(y), c);
        x += xInc;
        y += yInc;
    }
}

void Line::DrawLineMidpoint(int x1, int y1, int x2, int y2, COLORREF c) {
    bool steep = abs(y2 - y1) > abs(x2 - x1);
    if (steep) {
        swap(x1, y1);
        swap(x2, y2);
    }
    if (x1 > x2) {
        swap(x1, x2);
        swap(y1, y2);
    }

    int dx = x2 - x1;
    int dy = abs(y2 - y1);
    int d = 2 * dy - dx;
    int yInc = (y1 < y2) ? 1 : -1;
    int y = y1;

    for (int x = x1; x <= x2; ++x) {
        if (steep)
            SetPixel(hdc, y, x, c);
        else
            SetPixel(hdc, x, y, c);

        if (d > 0) {
            y += yInc;
            d -= 2 * dx;
        }
        d += 2 * dy;
    }
}

void Line::DrawLineParametric(int x1, int y1, int x2, int y2, COLORREF c) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = max(abs(dx), abs(dy));
    for (int i = 0; i <= steps; ++i) {
        float t = i / (float)steps;
        int x = round(x1 + dx * t);
        int y = round(y1 + dy * t);
        SetPixel(hdc, x, y, c);
    }
}

void Line::DrawLineInterpolated(int x1, int y1, int x2, int y2, COLORREF c1, COLORREF c2) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    if (abs(dx) >= abs(dy)) {
        if (x1 > x2) {
            swap(x1, x2);
            swap(y1, y2);
        }
        float m = (float)dy / dx;
        float y = y1;
        for (int x = x1; x <= x2; ++x) {
            float t = (float)(x - x1) / (x2 - x1);
            SetPixel(hdc, x, round(y), Mix(c1, c2, t));
            y += m;
        }
    } else {
        if (y1 > y2) {
            swap(x1, x2);
            swap(y1, y2);
        }
        float m = (float)dx / dy;
        float x = x1;
        for (int y = y1; y <= y2; ++y) {
            float t = (float)(y - y1) / (y2 - y1);
            SetPixel(hdc, round(x), y, Mix(c1, c2, t));
            x += m;
        }
    }
}

COLORREF Line::Mix(COLORREF c1, COLORREF c2, float t) {
    int r = GetRValue(c1) + t * (GetRValue(c2) - GetRValue(c1));
    int g = GetGValue(c1) + t * (GetGValue(c2) - GetGValue(c1));
    int b = GetBValue(c1) + t * (GetBValue(c2) - GetBValue(c1));
    return RGB(r, g, b);
}
